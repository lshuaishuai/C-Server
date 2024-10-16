#include "hook.h"
#include "fiber.h"
#include "iomanager.h"
#include "log.h"
#include "fd_manager.h"
#include "config.h"

#include <dlfcn.h>

shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system");

namespace shuai
{

static shuai::ConfigVar<int>::ptr g_tcp_connect_timeout = 
    shuai::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeoyt");

static thread_local bool t_hook_enable = false;   // 描述当前线程是否被hook

#define HOOK_FUNC(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

void hook_init()
{
    static bool is_inited = false;
    if(is_inited) return;

// 使用 dlsym(RTLD_NEXT, #name) 动态地从运行时链接的库解析出原始的函数地址
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
// HOOK_FUNC执行的为下面两行代码
// sleep_f = (sleep_fun)dlsym(RTLD_NEXT, "sleep");
// usleep_f = (usleep_fun)dlsym(RTLD_NEXT, "usleep");
    HOOK_FUNC(XX);
#undef XX
}

static uint64_t s_connect_timeout = -1;

struct _HookIniter
{
    _HookIniter()
    {
        hook_init();
        s_connect_timeout = g_tcp_connect_timeout->getValue();

        g_tcp_connect_timeout->addListener([](const int& old_value, const int& new_value){
                SHUAI_LOG_INFO(g_logger) << "tcp connect timeout changed from" << old_value << " to " << new_value;
                s_connect_timeout = new_value;
        });
    }
};

// 对于静态对象（如类的静态成员或全局对象），其构造函数会在所有全局变量被初始化后调用。
static _HookIniter s_hook_initer;    // 在进入main函数之前就将hook函数和声明的变量关联起来

bool is_hook_enable()
{
    return t_hook_enable;
}

void set_hook_enable(bool flag)
{
    t_hook_enable = flag;
}

}

struct timer_info
{
    int cancelled = 0;
};

// 传一个要hook的函数名 hook住了和io相关的操作
template<typename OriginFun, typename ... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name, uint32_t event, int timeout_so, Args&&... args)
{
    // 如果不需要hook则直接返回原函数
    if(!shuai::t_hook_enable) return fun(fd, std::forward<Args>(args)...);


    // fd不在fdCOntext中则直接返回原函数
    shuai::FdCtx::ptr ctx = shuai::FdMgr::GetInstance()->get(fd);
    if(!ctx) return fun(fd, std::forward<Args>(args)...);

    if(ctx->isColse())
    {
        // 检查文件描述符是否已经关闭，如果是，则设置错误码为 EBADF（坏文件描述符）
        errno = EBADF;
        return -1;
    }

    // 不是sock文件描述符或者用户设置了非阻塞
    if(!ctx->isSocket() || ctx->getUserNonblock()) return fun(fd, std::forward<Args>(args)...);

    // 拿到超时时间 
    uint64_t to = ctx->getTimeout(timeout_so);
    // 超时条件
    std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
    // std::forward<Args>(args)... 用于完美转发参数，确保在调用 fn 时保持参数的值类别（左值或右值）
    // 执行该函数
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    // EINTR: 当一个系统调用被信号中断时，通常会返回 -1，并将 errno 设置为 EINTR。这是一个常见的情况，尤其是在网络编程和文件操作中。例如，read、write 等系统调用都可能被信号中断
    // EINTR是可以重新读取
    while(n == -1 && errno == EINTR)
    {
        n = fun(fd, std::forward<Args>(args)...);
    }

    // 如果io操作返回值>0,如read就是读到数据了，可以直接return回去
    // 在使用非阻塞 I/O 时，返回值 -1 和 errno 设置为 EAGAIN 或 EWOULDBLOCK 表示当前没有可用的数据。这是一个正常的情况，说明操作暂时无法完成，但不意味着出错。
    // 没有数据，那么就可以将协程执行时间交出去 需要异步操作了
    if(n == -1 && errno == EAGAIN)
    {
        shuai::IOManager* iom = shuai::IOManager::GetThis();
        shuai::Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        // 如果 to 不是 -1，则添加一个条件定时器。当定时器超时后，会检查 timer_info 是否有效（未被取消），如果有效，则标记为超时，并取消相应的 I/O 事件
         if(to != (uint64_t)-1)
        {
            // 当到达超时时间后，就强制唤醒
            timer = iom->addConditionTimer(to, [winfo, fd, iom, event](){
                // 这行代码尝试从 winfo（弱指针）获取一个共享指针（shared_ptr）。lock() 方法会检查原始对象是否仍然存在（即 shared_ptr 的引用计数大于零）。
                // 如果对象仍然存在，lock() 会返回一个有效的 shared_ptr，将其赋值给 t。如果对象已经被销毁（即 shared_ptr 的引用计数为零），lock() 返回一个空的 shared_ptr。
                auto t = winfo.lock();
                if(!t || t->cancelled) return;
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, (shuai::IOManager::Event)(event));  // 继续执行该协程
            }, winfo);
        }

        // 添加事件 一当前协程为唤醒对象 
        int rt = iom->addEvent(fd, (shuai::IOManager::Event)(event));
        // 添加失败
        if(rt)
        {
            SHUAI_LOG_ERROR(g_logger) << hook_fun_name << "addEvent(" 
                                      << fd << ", " << event << ")";
            if(timer) timer->cancel();
            
            return -1;
        }
        else // 添加成功
        {
    // SHUAI_LOG_INFO(g_logger) << "do_io <" << hook_fun_name << ">";
            shuai::Fiber::YieldToHold();
    // SHUAI_LOG_INFO(g_logger) << "do_io <" << hook_fun_name << ">";
            // 从这里协程被唤醒回来有两种情况，一种是真的读到了事件，或者是定时器超时
            if(timer) timer->cancel();

            if(tinfo->cancelled) 
            {
                // 是通过定时任务被唤醒的 到这里就是事件取消了 不许到IO操作了
                errno = tinfo->cancelled;
                return -1;
            }
            // 说明有io时间来了，重新去读
            goto retry;
        }
    }
    return n;
}


extern "C"
{
// 在 extern "C" 块中定义的 sleep_f 和 usleep_f 是在程序的全局作用域中，它们的内存分配和初始化是在全局静态对象的构造之前完成的。
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUNC(XX);
#undef XX

unsigned int sleep(unsigned int seconds)
{
    if(!shuai::t_hook_enable) return sleep_f(seconds);

    shuai::Fiber::ptr fiber = shuai::Fiber::GetThis();
    shuai::IOManager* iom = shuai::IOManager::GetThis();
    iom->addTimer(seconds * 1000, std::bind((void(shuai::Scheduler::*)
            (shuai::Fiber::ptr, int thread))&shuai::IOManager::schedule
            , iom, fiber, -1));
    // iom->addTimer(seconds * 1000, [iom, fiber](){
    // // SHUAI_LOG_DEBUG(SHUAI_LOG_ROOT()) << "cur fiber id is = " << fiber->getId();
    //     iom->schedule(fiber);  // 将当前协程放到任务队列中，等定时器到了执行：也就是切换到当前协程继续执行
    // });
    // SHUAI_LOG_DEBUG(SHUAI_LOG_ROOT()) << "cur fiber id is = " << shuai::GetFiberId();
    shuai::Fiber::YieldToHold();
    // 时间到后，会切换到这里
    return 0;
}

int usleep(useconds_t usec)
{
    if(!shuai::t_hook_enable) return usleep_f(usec);

    shuai::Fiber::ptr fiber = shuai::Fiber::GetThis();
    shuai::IOManager* iom = shuai::IOManager::GetThis();
    iom->addTimer(usec / 1000, std::bind((void(shuai::Scheduler::*)
            (shuai::Fiber::ptr, int thread))&shuai::IOManager::schedule
            , iom, fiber, -1));
    // iom->addTimer(usec / 1000, [iom, fiber](){
        // iom->schedule(fiber);  // 设置定时器后就将该协程切走，但状态设置为HOLD，将来还要回来继续执行，等定时器到时后会将该协程添加到任务队列中，执行任务的协程会切换回该协程，继续执行完  用同步的写法实现了异步
    // });
    shuai::Fiber::YieldToHold();
    return 0;
}

int nanosleep(const struct timespec* req, struct timespec* rem)
{
    if(!shuai::t_hook_enable) return nanosleep_f(req, rem);

    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
    shuai::Fiber::ptr fiber = shuai::Fiber::GetThis();
    shuai::IOManager* iom = shuai::IOManager::GetThis();
    iom->addTimer(timeout_ms, std::bind((void(shuai::Scheduler::*)
            (shuai::Fiber::ptr, int thread))&shuai::IOManager::schedule
            , iom, fiber, -1));
    // iom->addTimer(timeout_ms, [iom, fiber](){
    //     iom->schedule(fiber);  // 设置定时器后就将该协程切走，但状态设置为HOLD，将来还要回来继续执行，等定时器到时后会将该协程添加到任务队列中，执行任务的协程会切换回该协程，继续执行完  用同步的写法实现了异步
    // });
    shuai::Fiber::YieldToHold();
    return 0;
}

int socket(int domain, int type, int protocol)
{
    if(!shuai::t_hook_enable) return socket_f(domain, type, protocol);

    int fd = socket_f(domain, type, protocol);
    if(fd == -1) return fd;

    shuai::FdMgr::GetInstance()->get(fd, true); // 若fd不存在且auto_create=true则新创建一个fd
    return fd;
}

/*等待超时或套接字可写，如果先超时，则条件变量winfo仍然有效，通过winfo来设置超时标志并触发WRITE事件，协程从yield点返回，
  返回之后通过超时标志设置errno并返回-1；如果在未超时之前套接字就可写了，那么直接取消定时器并返回成功。
  取消定时器会导致定时器回调被强制执行一次，但这并不会导致问题，因为只有当前协程结束后，定时器回调才会在接下来被调度，
  由于定时器回调被执行时connect_with_timeout协程已经执行完了，所以理所当然地条件变量也被释放了，所以实际上定时器回调函数什么也没做。
  这里是sylar条件定时器的巧妙应用，自行体会，感觉说得不是很清楚。*/
int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms)
{
    if(!shuai::t_hook_enable) return connect_f(fd, addr, addrlen);

    shuai::FdCtx::ptr ctx = shuai::FdMgr::GetInstance()->get(fd);
    if(!ctx || ctx->isColse()) 
    {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket()) return connect_f(fd, addr, addrlen);

    if(ctx->getUserNonblock()) return connect_f(fd, addr, addrlen);

    int n = connect_f(fd, addr, addrlen);
    if(n == 0) return 0;

    // 调用系统的connect函数，由于套接字是非阻塞的，这里会直接返回EINPROGRESS错误。
    else if(n != -1 || errno != EINPROGRESS) return n;

    shuai::IOManager* iom = shuai::IOManager::GetThis();
    shuai::Timer::ptr timer;
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);

    if(timeout_ms != (uint64_t)-1)
    {
        timer = iom->addConditionTimer(timeout_ms, [winfo, fd, iom]{
                auto t = winfo.lock();
                if(!t || t->cancelled) return;
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, shuai::IOManager::WRITE);  // 在定时时间到后通过t->cancelled设置超时标志并触发一次WRITE事件。
        }, winfo);
    }

    int rt = iom->addEvent(fd, shuai::IOManager::WRITE);
    if(rt == 0)
    {
        shuai::Fiber::YieldToHold();
        if(timer) timer->cancel();
        if(tinfo->cancelled) 
        {
            errno = tinfo->cancelled;
            return -1;
        }
    }
    else
    {
        if(timer) timer->cancel();
        SHUAI_LOG_ERROR(g_logger) << "connect addEvent (" << fd << ", WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) return -1;

    if(!error) return 0;
    else
    {
        errno = error;
        return -1;
    }
}

// int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
// {
//     return connect_with_timeout(sockfd, addr, addrlen, shuai::s_connect_timeout);
// }

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int fd = do_io(sockfd, accept_f, "accept", shuai::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if(fd >= 0) shuai::FdMgr::GetInstance()->get(fd, true);

    return fd;
}

// read
ssize_t read(int fd, void *buf, size_t count)
{
    return do_io(fd, read_f, "read", shuai::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
    return do_io(fd, readv_f, "readv", shuai::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    return do_io(sockfd, recv_f, "recv", shuai::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
    return do_io(sockfd, recvfrom_f, "recvfrom", shuai::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
{
    return do_io(sockfd, recvmsg_f, "recvmsg", shuai::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

// write
ssize_t write(int fd, const void *buf, size_t count)
{
    return do_io(fd, write_f, "write", shuai::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    return do_io(fd, writev_f, "writev", shuai::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    return do_io(sockfd, send_f, "send", shuai::IOManager::WRITE, SO_SNDTIMEO, buf, len, flags);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
    return do_io(sockfd, sendto_f, "sendto", shuai::IOManager::WRITE, SO_SNDTIMEO, buf, len, flags, dest_addr, addrlen);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
    return do_io(sockfd, sendmsg_f, "sendmsg", shuai::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd)
{
    if(shuai::t_hook_enable) return close_f(fd);

    shuai::FdCtx::ptr ctx = shuai::FdMgr::GetInstance()->get(fd);
    if(ctx)
    {
        auto iom = shuai::IOManager::GetThis();
        if(iom)
            iom->cancelAll(fd);  // 取消所有的事件
        shuai::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */ )
{
    va_list va;
    va_start(va, cmd);
    switch (cmd)
    {
    case F_SETFL:
        {
            int arg = va_arg(va, int);
            va_end(va);
            shuai::FdCtx::ptr ctx = shuai::FdMgr::GetInstance()->get(fd);
            if(!ctx || ctx->isColse() || !ctx->isSocket()) return fcntl_f(fd, cmd, arg);

            ctx->setUserNonblock(arg & O_NONBLOCK);
            if(ctx->getSysNonblock()) arg |= O_NONBLOCK;
            else arg &= ~O_NONBLOCK;

            return fcntl_f(fd, cmd, arg);
        }
        break;

    case F_GETFL:
        {
            va_end(va);
            int arg = fcntl_f(fd, cmd);
            shuai::FdCtx::ptr ctx = shuai::FdMgr::GetInstance()->get(fd);
            if(!ctx || ctx->isColse() || !ctx->isSocket()) return arg;

            if(ctx->getUserNonblock()) return arg | O_NONBLOCK;
            else return arg & ~O_NONBLOCK;
        }
        break;

    case F_DUPFD:
    case F_DUPFD_CLOEXEC:
    case F_SETFD:
    case F_SETOWN:
    case F_SETSIG:    
    case F_SETLEASE:    
    case F_NOTIFY:    
    case F_SETPIPE_SZ:
        {
            int arg = va_arg(va, int);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }    
        break;

    case F_GETFD: 
    case F_GETOWN:
    case F_GETSIG:
    case F_GETLEASE:
    case F_GETPIPE_SZ:
        {
            va_end(va);
            return fcntl_f(fd, cmd);
        }
        break;

    case F_SETLK:
    case F_SETLKW:
    case F_GETLK:
        {
            struct flock* arg = va_arg(va, struct flock*);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        break;

    case F_GETOWN_EX:
    case F_SETOWN_EX: 
        break;

    default:
        va_end(va);
        return fcntl_f(fd, cmd);
    }
    return 0;
}

int ioctl(int d, unsigned int long request, ...)
{
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);
    
    if(FIONBIO == request)
    {
        bool user_nonblock = !!*(int*)arg;
        shuai::FdCtx::ptr ctx = shuai::FdMgr::GetInstance()->get(d);
        if(!ctx || ctx->isColse() || !ctx->isSocket()) return ioctl_f(d, request, arg);

        ctx->setUserNonblock(user_nonblock);
    }
    return fcntl_f(d, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    if(!shuai::t_hook_enable) return setsockopt_f(sockfd, level, optname, optval, optlen);

    if(level == SOL_SOCKET)
    {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO)
        {
            shuai::FdCtx::ptr ctx = shuai::FdMgr::GetInstance()->get(sockfd);
            if(ctx)
            {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    // SHUAI_LOG_DEBUG(g_logger) << "sockfd = " << sockfd << " level = " << level << " optname = " << optname << " optval = " << optval << " optlen = " << optlen;
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}

}

