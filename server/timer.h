#pragma once

#include <memory>
#include <set>

#include "util.h"
#include "log.h"    
#include "thread.h"

namespace shuai
{

class TimerManager;
class Timer: public std::enable_shared_from_this<Timer>
{
    friend class TimerManager;
public:
    typedef std::shared_ptr<Timer> ptr;

    bool cancel();
    bool refresh();
    bool reset(uint64_t ms, bool from_now);

private:
    Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager* manager);
    Timer(uint64_t next);

private:
    bool m_recurring = false;               // 是否循环定时器 为true时，每次执行将cb交给IOManager时不会将任务彻底删除，而是重新添加到m_timers中
    uint64_t m_ms = 0;                      // 执行周期
    uint64_t m_next;                        // 精确的执行时间 下一次执行的时间
    std::function<void()> m_cb;             // 定时器执行的函数
    TimerManager* m_manager = nullptr;

private:
    struct Comparator
    {
        bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
    };
};

class TimerManager
{
    friend class Timer;
public:
    typedef RWMutex RWMutexType;

    TimerManager();
    virtual ~TimerManager();

    Timer::ptr addTimer(uint64_t ms, std::function<void()>, bool recurring = false);
    Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring = false); 

    uint64_t getNextTimer();
    void listExpiredCb(std::vector<std::function<void()>>& cbs);
protected:
    virtual void onTimerInsertedAtFront() = 0;           // 当有新的定时器插入到定时器的首部,执行该函数
    void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);
    bool hasTimer();

private:
    bool detectClockRollover(uint64_t now_ms);           // 检测服务器时间是否被调后了

private:
    RWMutexType m_mutex;
    std::set<Timer::ptr, Timer::Comparator> m_timers;    // m_timers 中的元素将按 m_next 值从小到大排列
    bool m_tickled = false;                              // 是否触发onTimerInsertedAtFront
    uint64_t m_previouseTime = 0;                        // 上次执行时间
};

}