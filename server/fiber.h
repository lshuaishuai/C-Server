#pragma once

#include <memory>
#include <functional>
#include <ucontext.h>

#include "thread.h"
#include "log.h"
// #include "scheduler.h"

namespace shuai
{

class Scheduler;
// 继承自 std::enable_shared_from_this<Fiber>，使得 Fiber 对象可以安全地创建 shared_ptr 实例
class Fiber: public std::enable_shared_from_this<Fiber>
{
    friend class Scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;

    enum State{
        INIT,  // 初始化状态。协程刚被创建，但尚未开始执行
        HOLD,  // "保持状态"（Hold State）通常指的是协程在执行过程中暂时停止，但并未结束其生命周期。此时，协程的上下文仍然被保留，以便可以稍后恢复执行
        EXEC,  // 执行状态。协程正在被调度和执行，可以进行任务处理
        TERM,  // 终止状态。协程已经完成其任务并正常结束，不再可执行
        READY, // 就绪状态。协程已准备好执行，但可能因为调度优先级或其他因素而尚未开始运行
        EXCEPT // 异常状态。协程在执行过程中遇到了错误或异常，导致其无法继续正常运行
    };

private:
    Fiber();  // 默认构造函数就是线程的主协程，无参数

public:
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);   // 这是真正的开创了一个协程，需要分配栈空间
    ~Fiber();

    void reset(std::function<void()> cb);   // 协程执行完或者有问题时，重新设置协程函数并重置状态，省了内存的分配和释放
    void swapIn();                          // 自己开始执行，切换到当前协程执行
    void swapOut();                         // 让出执行权  切换到后台
    void call();
    void back();

    uint64_t getId() { return m_id; }
    void setState(State state) { m_state = state; }
    State getState() const { return m_state; }

public:
    static void SetThis(Fiber* f);          // 设置当前协程
    static Fiber::ptr GetThis();            // 获取当前协程
    static void YieldToReady();             // 协程切换到后台，并且设置为Ready状态
    static void YieldToHold();              // 协程切换到后台，并且设置为Hold状态
    static uint64_t TotalFibers();          // 总协程数

    static void MainFunc();
    static void CallerMainFunc();
    static uint64_t GetFiberId();

private:
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;         // 协程栈大小
    State m_state = INIT;         // 状态
    
    ucontext_t m_ctx;
    void* m_stack = nullptr;

    std::function<void()> m_cb;   // 执行的协程方法 
};

}