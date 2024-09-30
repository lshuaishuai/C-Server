#pragma once

#include <memory>
#include <functional>
#include <ucontext.h>

#include "thread.h"
#include "log.h"

namespace shuai
{

// 继承自 std::enable_shared_from_this<Fiber>，使得 Fiber 对象可以安全地创建 shared_ptr 实例
class Fiber: public std::enable_shared_from_this<Fiber>
{
public:
    typedef std::shared_ptr<Fiber> ptr;

    enum State{
        INIT,
        HOLD,  // "保持状态"（Hold State）通常指的是协程在执行过程中暂时停止，但并未结束其生命周期。此时，协程的上下文仍然被保留，以便可以稍后恢复执行
        EXEC,
        TERM,
        READY,
        EXCEPT
    };

private:
    Fiber();  // 默认构造函数就是线程的主协程，无参数

public:
    Fiber(std::function<void()> cb, size_t stacksize = 0);   // 这是真正的开创了一个协程，需要分配栈空间
    ~Fiber();

    void reset(std::function<void()> cb);   // 协程执行完或者有问题时，重新设置协程函数并重置状态，省了内存的分配和释放
    void swapIn();                          // 自己开始执行，切换到当前协程执行
    void swapOut();                         // 让出执行权  切换到后台
    uint64_t getId() { return m_id; }

public:
    static void SetThis(Fiber* f);          // 设置当前协程
    static Fiber::ptr GetThis();            // 获取当前协程
    static void YieldToReady();             // 协程切换到后台，并且设置为Ready状态
    static void YieldToHold();              // 协程切换到后台，并且设置为Hold状态
    static uint64_t TotalFibers();          // 总协程数

    static void MainFunc();
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