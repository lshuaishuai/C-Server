#pragma once

#include <memory>
#include <atomic>
#include <vector>
#include <list>

#include "fiber.h"
#include "thread.h"

namespace shuai
{

class Scheduler
{
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");  // use_caller为true则将调度器所在的线程纳入线程调度器中
    virtual ~Scheduler();

    const std::string& getName() const { return m_name; }

    static Scheduler* GetThis();      // 得到当前的协程调度器是哪个
    static Fiber* GetMainFiber();     // 每个调度器需要一个主协程

    void start();
    void stop();
    
    template <class FiberOrCb>
    void schedule(FiberOrCb fc, int thread = -1)
    {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thread);
        }
        
        if(need_tickle)
        {
            tickle();
        }
    }

    template <class InputIterator>
    void schedule(InputIterator begin, InputIterator end)
    {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin != end)
            {
                need_tickle = scheduleNoLock(&*begin) || need_tickle;
                ++begin;
            }
        } 
        if(need_tickle)
        {
            tickle();
        }
    }

protected:
    virtual void tickle();     // tickle方法 通知各调度线程的调度协程有新任务来了
    void run();
    virtual bool stopping();
    virtual void idle();       // 解决协程调度器无任务做时，又不能使线程终止

    void setThis();

private:
    // FiberOrCb类型可以有两种：Fiber、std::function<void()>
    template <class FiberOrCb>  
    bool scheduleNoLock(FiberOrCb fc, int thread)
    {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, thread);
        if(ft.fiber || ft.cb)   
            m_fibers.push_back(ft);
        return need_tickle;
    }   

private:
    /**
     * @brief 调度任务，协程/函数二选一，可指定在哪个线程上调度
     */
    struct FiberAndThread
    {
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread;

        FiberAndThread(Fiber::ptr f, int thr)
            :fiber(f)
            ,thread(thr) 
        {}

        FiberAndThread(Fiber::ptr* f, int thr)
            :thread(thr)
        {
            fiber.swap(*f);   // 传进来swap之后，f就为nullptr了，不会增加引用计数
        }

        FiberAndThread(std::function<void()> c, int thr)
            :cb(c)
            ,thread(thr)
        {}

        FiberAndThread(std::function<void()>* c, int thr)
            :thread(thr)
        {
            cb.swap(*c);
        }

        FiberAndThread()
            :thread(-1)
        {}

        void reset()
        {
            thread = -1;
            fiber = nullptr;
            cb = nullptr;
        }
    };
    

private:
    MutexType m_mutex;
    std::vector<Thread::ptr> m_threads;
    std::list<FiberAndThread> m_fibers;   // 存放即将要执行、计划要执行的协程
    Fiber::ptr m_rootFiber;               // use_caller为true时，调度器所在线程的调度协程
    std::string m_name;                  

protected:
    std::vector<int> m_threadIds; // 存线程id
    size_t m_threadCount = 0;
    std::atomic<size_t> m_activateThreadCount = {0};
    std::atomic<size_t> m_idleThreadCount = {0};    // 空闲线程
    bool m_stopping = true;          // 停止
    bool m_autoStop = false;         // 是否主动停止
    int m_rootThread = 0;            // use_caller为true时，调度器所在线程的id
};

} // namespace shuai