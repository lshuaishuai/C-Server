#pragma once

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#include "macro.h"
#include "log.h"
#include "scheduler.h"
#include "timer.h"

namespace shuai
{

class IOManager: public Scheduler, public TimerManager
{
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;

    enum Event
    {
        NONE  = 0x0,
        READ  = 0x1,    // EPOLLIN
        WRITE = 0x4,    // EPOLLOUT
    };
private:
    // 该结构体表示事件使用来做什么的？
    /**
     * @brief socket fd上下文类
     * @details 每个socket fd都对应一个FdContext，包括fd的值，fd上的事件，以及fd的读写事件上下文
     */
    struct FdContext
    {
        typedef Mutex MutexType;
        /**
         * @brief 事件上下文类
         * @details fd的每个事件都有一个事件上下文，保存这个事件的回调函数以及执行回调函数的调度器
         *          sylar对fd事件做了简化，只预留了读事件和写事件，所有的事件都被归类到这两类事件中
         */
        struct EventContext
        {
            Scheduler* scheduler = nullptr;  // 表示事件在哪一个scheduler上去执行
            Fiber::ptr fiber;                // 事件的协程
            std::function<void()> cb;        // 事件的回调
        };

        EventContext& getContext(Event event);
        void resetContext(EventContext& ctx);
        void triggerEvent(Event event);

        EventContext read;     // 读事件
        EventContext write;   // 写事件
        int fd;                // epoll事件关联的句柄
        Event events = NONE;   // 已注册的事件
        MutexType mutex;       // 互斥锁
    };

public:
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    ~IOManager();

    // 1.success 0.retry -1.error
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event event);      // 删除事件
    bool cancelEvent(int fd, Event event);   // 取消事件
    bool cancelAll(int fd);

    static IOManager* GetThis();             // 获取当前的IOManager
    
protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;
    void onTimerInsertedAtFront() override;
    bool stopping(uint64_t& timeout);

    void contextResize(size_t size);
private:
    int m_epfd = 0;      // epoll的句柄
    int m_tickleFds[2];  // pipe文件句柄 0为读端，1为写端

    std::atomic<size_t> m_pendingEventCount = {0};  // 现在等待执行的事件数量
    RWMutexType m_mutex;
    std::vector<FdContext*> m_fdContexts;           // socket事件上下文的容器
};

}