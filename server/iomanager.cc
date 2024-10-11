#include "iomanager.h"

#include <errno.h>
#include <string.h>

namespace shuai
{

static shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system");

IOManager::FdContext::EventContext& IOManager::FdContext::getContext(IOManager::Event event)
{
    switch (event)
    {
    case IOManager::READ:
        return read;
    case IOManager::WRITE:
        return write;
    default:
        SHUAI_ASSERT2(false, "getContext");
    }
}

void IOManager::FdContext::resetContext(EventContext& ctx)
{
    ctx.scheduler = nullptr;
    ctx.fiber.reset();
    ctx.cb = nullptr;
}

/**
* @brief 触发事件
* @details 根据事件类型调用对应上下文结构中的调度器去调度回调协程或回调函数
* @param[in] event 事件类型
*/
void IOManager::FdContext::triggerEvent(IOManager::Event event)
{
    // SHUAI_LOG_DEBUG(g_logger) << "triggerEvent";
    SHUAI_ASSERT(events & event);
    events = (Event)(events & ~event);
    EventContext& ctx = getContext(event);

    if(ctx.cb)
    {
        ctx.scheduler->schedule(&ctx.cb);
    }
    else{
        ctx.scheduler->schedule(&ctx.fiber);
    }
    ctx.scheduler = nullptr;
    return;
}

IOManager::IOManager(size_t threads, bool use_caller, const std::string& name)
    :Scheduler(threads, use_caller, name)
{
    m_epfd = epoll_create(5000);
    SHUAI_ASSERT(m_epfd > 0);

    // 创建pipe，获取m_tickleFds[2]，其中m_tickleFds[0]是管道的读端，m_tickleFds[1]是管道的写端
    int rt = pipe(m_tickleFds);
    SHUAI_ASSERT(!rt);

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;        // 边缘触发

    event.data.fd = m_tickleFds[0];   

    // 设置为非阻塞
    rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
    SHUAI_ASSERT(!rt);

    // 将管道的读描述符加入epoll多路复用，如果管道可读，idle中的epoll_wait会返回
    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
    SHUAI_ASSERT(!rt);

    contextResize(32);

    // 这里直接开启了Schedluer，也就是说IOManager创建即可调度协程
    start();
}

IOManager::~IOManager()
{
    // SHUAI_LOG_DEBUG(g_logger) << "~IOManager()";
    // sleep(1000);
    stop();
    // SHUAI_LOG_DEBUG(g_logger) << "have stopped";

    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);

    for(size_t i = 0;  i < m_fdContexts.size(); ++i)
        if(m_fdContexts[i]) delete m_fdContexts[i];
}

void IOManager::contextResize(size_t size)
{
    m_fdContexts.resize(size);
    for(size_t i = 0; i < m_fdContexts.size(); ++i)
    {
        if(!m_fdContexts[i])
        {
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->fd = i; 
        }
    }
}

// 返回值：1.success 0.retry -1.error 往sock中添加事件
int IOManager::addEvent(int fd, Event event, std::function<void()> cb)
{
    FdContext* fd_ctx = nullptr;
    // 找到fd对应的FdContext，如果不存在，那就分配一个
    RWMutexType::WriteLock lock(m_mutex);
    if((int)m_fdContexts.size() > fd)
    {
        fd_ctx = m_fdContexts[fd];
        lock.unlock();
    }
    else
    {
        lock.unlock();
        RWMutexType::WriteLock lock2(m_mutex);
        contextResize(fd * 1.5);
        fd_ctx = m_fdContexts[fd];
    }
    // SHUAI_LOG_DEBUG(g_logger) << "向fdcontext中添加完成";
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    // 当 fd_ctx->events 中包含 event 的位时，即 fd_ctx->events & event 的结果不为零。这意味着相应的事件是被设置的
    // 如果 fd_ctx->events 中不包含 event 的位，那么该表达式的值为0，表示该事件未被设置
    if(fd_ctx->events & event)
    {
        // 一个线程不可能添加两次同样的事件，若是遇到这种情况表明多个线程同时操作，直接断言
        // 如果一个线程在加锁之前已经添加了某个事件，而另一个线程在此之后尝试添加相同的事件，就会出现问题。
        SHUAI_LOG_ERROR(g_logger) << "addEvent assert fd = " << fd
                                  << " event = " << event
                                  << " fd_ctx.event = " << fd_ctx->events;
        SHUAI_ASSERT(!(fd_ctx->events & event));  
    }

    // 将新的事件加入epoll，使用epoll_event的私有指针存储FdContext的位置
    // 当fd_ctx注册了读或写事件时
    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;  
    epoll_event epevent;
    epevent.events = EPOLLET | fd_ctx->events | event;  // 注册新事件
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt)
    {
        SHUAI_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << op << ", " 
                                  << fd << ", " << epevent.events << "):" 
                                  << rt << "(" << errno << ") (" << strerror(errno) << ")";
        return -1;
    }
    // SHUAI_LOG_DEBUG(g_logger) << "向epoll中注册fd对应的新事件";

    ++m_pendingEventCount;
    // 找到这个fd的event事件对应的EventContext，对其中的scheduler, cb, fiber进行赋值
    fd_ctx->events = (Event)(fd_ctx->events | event);
    // 获取fd_ctx事件event的上下文 注意这里为引用
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    SHUAI_ASSERT(!event_ctx.scheduler && !event_ctx.fiber && !event_ctx.cb);

    // 赋值scheduler和回调函数，如果回调函数为空，则把当前协程当成回调执行体
    event_ctx.scheduler = Scheduler::GetThis();
    if(cb)
    {
        event_ctx.cb.swap(cb);
    }
    else
    {
        event_ctx.fiber  = Fiber::GetThis();
        SHUAI_ASSERT(event_ctx.fiber->getState() == Fiber::EXEC);
    }
    return 0;
}

// 删除事件
bool IOManager::delEvent(int fd, IOManager::Event event)      
{
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd)
        return false;
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    // 未注册该event事件 直接返回
    if(!(fd_ctx->events & event))
        return false;

    // 新的事件设置为从fd_ctx->events中删除event剩余的事件
    Event new_event = (Event)(fd_ctx->events & ~event);
    // 是否还有别的事件，没有就直接删除，有就为修改
    int op = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_event;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt)
    {
        SHUAI_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << op << ", " 
                                  << fd << ", " << epevent.events << "):" 
                                  << rt << "(" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    --m_pendingEventCount;
    fd_ctx->events = new_event;
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    // 将删除的事件所对应的事件上下文设置为nullptr
    fd_ctx->resetContext(event_ctx);
    return true;
}

// 取消事件: 跟删除差不多，区别在于
bool IOManager::cancelEvent(int fd, IOManager::Event event)   
{
    // SHUAI_LOG_DEBUG(g_logger) << "取消读事件";
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd)
        return false;
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!(fd_ctx->events & event))
        return false;

    Event new_event = (Event)(fd_ctx->events & ~event);
    int op = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_event;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt)
    {
        SHUAI_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << op << ", " 
                                  << fd << ", " << epevent.events << "):" 
                                  << rt << "(" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    // 取消之前触发一次事件
    fd_ctx->triggerEvent(event);
    --m_pendingEventCount;    
    // SHUAI_LOG_DEBUG(g_logger) << "取消读事件完成";
    return true;
}

bool IOManager::cancelAll(int fd)
{
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd)
        return false;
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!fd_ctx->events)
        return false;

    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt)
    {
        SHUAI_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << op << ", " 
                                  << fd << ", " << epevent.events << "):" 
                                  << rt << "(" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    // 将读事件和写事件都取消
    if(fd_ctx->events & READ)
    {
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;  
    }
    if(fd_ctx->events & WRITE)
    {
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;  
    }

    SHUAI_ASSERT(fd_ctx->events == 0);
    return true;
}

// 获取当前的IOManager
IOManager* IOManager::GetThis()             
{
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void IOManager::tickle()
{
    // SHUAI_LOG_DEBUG(g_logger) << "IOManager::tickle";
    if(!hasIdleThreads()) 
    {
        // SHUAI_LOG_DEBUG(SHUAI_LOG_ROOT()) << "idle Thread is: " << m_idleThreadCount;
        return;
    }
    // 往管道中写数据，表示有数据到来
    
    // SHUAI_LOG_DEBUG(SHUAI_LOG_ROOT()) << "write pipeline";
    int rt = write(m_tickleFds[1], "T", 1);
    SHUAI_ASSERT(rt == 1);
}

bool IOManager::stopping(uint64_t& timeout)
{
    timeout = getNextTimer();
    return timeout == ~0ull && m_pendingEventCount == 0 && Scheduler::stopping();
}

bool IOManager::stopping()
{
    uint64_t timeout = 0;
    return stopping(timeout);
}

// 先检查epoll中有哪些事件需要唤醒，处理完这些事件后让出执行权
// 实际是idle协程只负责收集所有已触发的fd的回调函数并将其加入调度器的任务队列，真正的执行时机是idle协程退出后，调度器在下一轮调度时执行
void IOManager::idle()
{
    SHUAI_LOG_INFO(g_logger) << "IOManager::idle()";
    epoll_event* events = new epoll_event[64]();
    // 使用 std::shared_ptr 管理这个数组的内存，确保在结束时自动释放
    std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr){
        delete[] ptr;
    });

    while(true)
    {
        uint64_t next_timeout = 0;
        if(stopping(next_timeout))
        {
            SHUAI_LOG_INFO(g_logger) << "name = " << getName() << " idle stopping exit";
            break;
        }

        int rt = 0;
        do{
            static const int MAX_TIMEOUT = 3000;    // 5s
            if(next_timeout != ~0ull)
            {
                next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
            }
            else{
                next_timeout = MAX_TIMEOUT;
            }
            // 调用 epoll_wait 函数阻塞，直到有事件发生或超时（5秒）
            // SHUAI_LOG_DEBUG(SHUAI_LOG_ROOT()) << "epoll_wait";
            rt = epoll_wait(m_epfd, events, 64, (int)next_timeout);  // 有新的事件添加或者删除就break
            // SHUAI_LOG_DEBUG(SHUAI_LOG_ROOT()) << "rt = " << rt;
            if(rt < 0 && (errno == EINTR)) {}
            else break; // epoll_wait超时或者等待到事件都会结束循环
        }while(true);

        std::vector<std::function<void()>> cbs;
        // 从这儿的到定时器的任务 并添加到任务队列
        listExpiredCb(cbs);
        if(!cbs.empty()) 
        {
            schedule(cbs.begin(), cbs.end());
            cbs.clear();
        }

        // 在该for里面进行了tickle 然后往pipe中写入了数据，以此来触发epoll wait的m_tickleFds[0]，等待到事件，然后执行
        for(int i = 0; i < rt; ++i)
        {
            epoll_event& event = events[i];
            // ticklefd[0]用于通知协程调度，这时只需要把管道里的内容读完即可，本轮idle结束Scheduler::run会重新执行协程调度
            if(event.data.fd == m_tickleFds[0])
            {
                uint8_t dummy;
                // 是让快点从epollwait返回，然后空闲协程去执行新的任务
                while(read(m_tickleFds[0], &dummy, 1) == 1);   // 当有新任务加入队列，tickle的时候会向管道中写数据，epoll监听到后返回，将空闲线程从idle中返回到run中执行新添加的任务
                continue;
            }

            FdContext* fd_ctx = (FdContext*)event.data.ptr;
            FdContext::MutexType::Lock lock(fd_ctx->mutex);
            // 当发生错误或挂起时，标记该事件为可读和可写，以便后续处理
            /**
             * EPOLLERR: 出错，比如写读端已经关闭的pipe
             * EPOLLHUP: 套接字对端关闭
             * 出现这两种事件，应该同时触发fd的读和写事件，否则有可能出现注册的事件永远执行不到的情况
             */

            if(event.events & (EPOLLERR | EPOLLHUP))
            {
                event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
            }
            int real_events = NONE;
            if(event.events & EPOLLIN)
                real_events |= READ;
            if(event.events & EPOLLOUT)
                real_events |= WRITE;

            if((fd_ctx->events & real_events) == NONE) continue;

            // 处理已经发生的事件，也就是让调度器调度指定的函数或协程
            int left_events = (fd_ctx->events & ~real_events);
            int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_events;
    
            int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
            if(rt2)
            {
                SHUAI_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << op << ", " 
                                << fd_ctx->fd << ", " << event.events << "):" 
                                << rt2 << "(" << errno << ") (" << strerror(errno) << ")";
                continue;
            }

            // 处理已经发生的事件，也就是让调度器调度指定的函数或协程 在trigger中tickle
            if(real_events & READ)
            {
        // SHUAI_LOG_INFO(g_logger) << "处理新添加的读事件";
                fd_ctx->triggerEvent(READ);
                --m_pendingEventCount;    
            }
            if(real_events & WRITE)
            {
        // SHUAI_LOG_INFO(g_logger) << "处理新添加的写事件";
                fd_ctx->triggerEvent(WRITE);
                --m_pendingEventCount;    
            }
        }
        /**
         * 一旦处理完所有的事件，idle协程yield，这样可以让调度协程(Scheduler::run)重新检查是否有新任务要调度
         * 上面triggerEvent实际也只是把对应的fiber重新加入调度，要执行的话还要等idle协程退出
         */
        Fiber::ptr cur = Fiber::GetThis();
        auto raw_ptr = cur.get();
        cur.reset();

        raw_ptr->swapOut();  // 让出执行权  第二次swapIn是到了这里了 还是在while里面
    }
}

void IOManager::onTimerInsertedAtFront()
{
    SHUAI_LOG_INFO(g_logger) << "addtimer";
    tickle();
}

}