#include "scheduler.h"
#include "log.h"
#include "macro.h"

namespace shuai
{
static shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system");   // 将自己库的函数功能都放到system Logger中

static thread_local Scheduler* t_scheduler = nullptr;   // 协程调度器指针
static thread_local shuai::Fiber* t_scheduler_fiber = nullptr;    // 当前线程的调度协程，每个线程都独有一份，包括caller线程  Fiber::t_fiber指向当前协程、Fiber::t_schedule_fiber指向主协程

// 是否把创建协程调度器的线程放入到调度器管理的线程池中。放入：将协程调度器封装到一个写成中，称之为主协程或调度器协程；不放入：这个线程专职协程调度
// use_caller为true则将该线程纳入线程调度器中
Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)  
    :m_name(name)
{
    SHUAI_ASSERT(threads > 0);

    if(use_caller)    // 将该线程纳入协程调度器中
    {
        shuai::Fiber::GetThis();     // 创建主协程，不是调度协程
        --threads;

        SHUAI_ASSERT(GetThis() == nullptr);   // 此时应该没有协程调度器
        t_scheduler = this;
        /**
         * 在user_caller为true的情况下，初始化caller线程的调度协程
         * caller线程的调度协程不会被调度器调度，而且，caller线程的调度协程停止时，应该返回caller线程的主协程
         */
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));  // use_caller为true时，调度器所在线程的调度协程 切换到n_rootFiber时会自动执行run函数
        shuai::Thread::SetName(m_name);

        t_scheduler_fiber = m_rootFiber.get();  // 在线程下声明一个协程调度器，再将当前线程放到调度器中，它的主协程不是线程的主协程而是执行run的主协程
        m_rootThread = shuai::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    }
    else
    {
        m_rootThread = -1;
    }
    m_threadCount = threads;
}

Scheduler::~Scheduler()
{
    SHUAI_ASSERT(m_stopping);
    if(GetThis() == this)
    {
        t_scheduler = nullptr;
    }
}

// 得到当前的协程调度器是哪个
Scheduler* Scheduler::GetThis()     
{
    return t_scheduler;
} 

// 每个调度器需要一个主协程
Fiber* Scheduler::GetMainFiber()
{
    return t_scheduler_fiber;
}    

void Scheduler::start()
{
    MutexType::Lock lock(m_mutex);
    if(!m_stopping)
    {
        return; // 未启动
    }
    m_stopping = false;
    SHUAI_ASSERT(m_threads.empty());  // 确保线程池是空

    // 这里为创建调度线程池
    m_threads.resize(m_threadCount);
    // 若是将创建调度器的线程也加入线程池且只有一个线程，那么该线程不仅创建调度器还要调度协程
    for(size_t i = 0; i < m_threadCount; ++i)
    {
        // SHUAI_LOG_DEBUG(g_logger) << "create schedule thread";
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();

    // if(m_rootFiber) 
    // {
    //     // m_rootFiber->swapIn();  // 调度协程并没有创建子协程啊 这里自己swapIn自己了
    //     m_rootFiber->call();     // 这一行已经进入了执行run方法的协程中去了
    //     SHUAI_LOG_INFO(g_logger) << "call out " << m_rootFiber->getState();
    // }
}

void Scheduler::stop()
{
    m_autoStop = true;
    if(m_rootFiber && m_threadCount == 0 && (m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::INIT))
    {
        SHUAI_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;

        if(stopping())
            return;
    }
    // SHUAI_LOG_DEBUG(g_logger) << "not execute return";
    // bool exit_on_this_fiber = false;
    if(m_rootThread != -1)
    {
        SHUAI_ASSERT(GetThis() == this);
    }
    else
    {
        SHUAI_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i)
    {
        tickle();  // 唤醒线程
    }

    if(m_rootFiber)
    {
        tickle();
    }

    if(m_rootFiber)
    {
        // while(!stopping())
        // {
        //     if(m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::EXCEPT)
        //     {
        //         m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        //         SHUAI_LOG_INFO(g_logger) << " root fiber is term, reset";
        //         t_scheduler_fiber = m_rootFiber.get();
        //     }
        //     m_rootFiber->call();

        // }
        if(!stopping())
        {
            m_rootFiber->call();   // 调度协程执行该函数
        }   
    }

    std::vector<Thread::ptr> thrs;
    {
        Mutex::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }
    for(auto& i : thrs)
    {
        i->join();
    }

    // if(exit_on_this_fiber)
    // {

    // }
}

void Scheduler::setThis()
{
    t_scheduler = this;
}

// 每个被调度器执行的协程在结束时都会回到调度协程
void Scheduler::run()
{
    SHUAI_LOG_DEBUG(g_logger) << "run";
    setThis();
    if(shuai::GetThreadId() != m_rootThread) // 当前线程不是创建协程调度器的线程(caller线程)则将调度协程设置为当前的协程
    {
        // use_caller==1, 且当只有一个线程时，不会走这里
        t_scheduler_fiber = Fiber::GetThis().get();  // 每个线程在执行run时都会创建自己的主协程，"但是一个调度器只有一个调度协程id为1,这句话对吗？"
    }

    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));    // 线程空闲时绑定空闲所对应的函数
    Fiber::ptr cb_fiber;

    FiberAndThread ft;  // 存放任务队列中的内容
    while(true)
    {
        ft.reset();                     // reset()将ft里面的内容全部置为nullptr
        bool tickle_me = false;         // 是否tickle其他线程进行任务调度
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            // 从消息队列中取出要执行的任务
            while(it != m_fibers.end())
            {
                // 抢到该任务的线程并不一定是当前的线程 通知其他线程
                if(it->thread != -1 && it->thread != shuai::GetThreadId())
                {
                    ++it;
                    tickle_me = true;   // 需要提醒其他线程进行任务调度 
                    continue;
                }   
                // 找到一个未指定线程或者是指定了当前线程的任务
                SHUAI_ASSERT(it->fiber || it->cb);
                // 这里可能有问题，任务队列的协程应该是READY状态吧？会把一个EXEC状态的协程放入队列啊
                if(it->fiber && it->fiber->getState() == Fiber::EXEC)   // 当前任务队列中的协程正在执行状态
                {
                    ++it;
                    continue;
                }

                // 这里拿到任务后不需要break吗？
                ft = *it;   // 从任务队列中取出任务后
                m_fibers.erase(it);
                ++m_activateThreadCount;
                is_active = true;
                break;
            }
        }

        if(tickle_me)   // 需要提醒其他线程进行任务调度 
        {
            tickle();
        }
        // 任务队列中为协程 并且改协程是可以执行的状态
        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM || ft.fiber->getState() != Fiber::EXCEPT))
        {
            ft.fiber->swapIn();  // 执行
            --m_activateThreadCount;
            
            // 下面是什么意思呢？
            if(ft.fiber->getState() == Fiber::READY)
            {
                schedule(ft.fiber);
            }
            // 若是协程切换回来以后，任务协程不处于终止或者异常状态，那么改写成并未执行完，设置为HOLD状态
            else if((ft.fiber->getState() != Fiber::TERM) && (ft.fiber->getState() != Fiber::EXCEPT))
            {
                ft.fiber->setState(Fiber::HOLD);
            }
            ft.reset();
        }
        else if(ft.cb)
        {
            // 用回调协程来执行任务队列中的函数任务？
            if(cb_fiber)
            {
                cb_fiber->reset(ft.cb);
            }
            else
            {
                // 在这里创建执行函数任务的协程
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            // SHUAI_LOG_INFO(g_logger) << "before execute cb task, return main fiber, fiber_id = " << GetFiberId() << "cb task state: " << cb_fiber->getState(); 
            cb_fiber->swapIn();    // 让回调协程执行函数任务
            // SHUAI_LOG_INFO(g_logger) << "after executed cb task, return main fiber, fiber_id = " << GetFiberId() << "cb task state: " << cb_fiber->getState(); 
            --m_activateThreadCount;

            // 下面是什么？-- 若该任务还处于READY状态，再将其放入任务队列？
            if(cb_fiber->getState() == Fiber::READY)
            {
                schedule(cb_fiber);
                cb_fiber.reset();
            }
            else if(cb_fiber->getState() == Fiber::EXCEPT || cb_fiber->getState() == Fiber::TERM)
            {
                // SHUAI_LOG_INFO(g_logger) << "executed cb task"; 
                cb_fiber->reset(nullptr);
                // SHUAI_LOG_INFO(g_logger) << "reseted cb task"; 
            }
            else // } if(cb_fiber->getState() != Fiber::TERM)
            {
                cb_fiber->setState(Fiber::HOLD);
                cb_fiber.reset();
            }
        }
        else  // 知道没有任务时执行idle 线程空闲
        {
            if(is_active)
            {
                --m_activateThreadCount;
                continue;
            }
            if(idle_fiber->getState() == Fiber::TERM)
            { 
                SHUAI_LOG_INFO(g_logger) << "idle fiber term";
                break;
                // continue;
            }
            ++m_idleThreadCount;  // 
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if(idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCEPT)
            {
                idle_fiber->setState(Fiber::HOLD);
            }
        }
    }
}

// tickle方法 通知各调度线程的调度协程有新任务来了
void Scheduler::tickle()
{
    SHUAI_LOG_INFO(g_logger) << "tickle";
}     

// 指示任务是否已经完成
bool Scheduler::stopping()
{
    MutexType::Lock lock(m_mutex); 
    return m_autoStop && m_stopping && m_fibers.empty() && (m_activateThreadCount == 0);
}

// 解决协程调度器无任务做时，又不能使线程终止
void Scheduler::idle()
{
    SHUAI_LOG_INFO(g_logger) << "idle";
    while(!stopping())
    {
        // SHUAI_LOG_DEBUG(g_logger) << "!stopping()";
        shuai::Fiber::YieldToHold();
    }
}       

}