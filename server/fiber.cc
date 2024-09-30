#include "fiber.h"
#include "config.h"
#include "macro.h"

#include <atomic>

namespace shuai
{

static shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

// 每个线程有一个主协程 这里表示的是当前的协程还是主协程？不太确定 
static thread_local Fiber* t_fiber = nullptr;                        // 保存当前正在运行的协程指针，必须时刻指向当前正在运行的协程对象。协程模块初始化时，t_fiber指向线程主协程对象
static thread_local Fiber::ptr t_threadFiber = nullptr;  // main协程 // 保存线程主协程指针，智能指针形式。协程模块初始化时，t_thread_fiber指向线程主协程对象。当子协程resume时，通过swapcontext将主协程的上下文保存到t_thread_fiber的ucontext_t成员中，同时激活子协程的ucontext_t上下文。当子协程yield时，从t_thread_fiber中取得主协程的上下文并恢复运行

static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
    Config::Lookup<uint32_t>("fiber.stack_size", 1024*1024*1, "fiber stack size");

// 实现了一个简单的内存分配器，使用 malloc 和 free 来进行内存的分配和释放
class MallocStackAllocator
{
public:
    static void* Alloc(size_t size)
    {
        return malloc(size); 
    }
    static void Dealloc(void* vp, size_t size)
    {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

// 这个方法专门用于获取主协程的 ID，而不是任意协程的 ID
uint64_t Fiber::GetFiberId()
{
    if(t_fiber)
        return t_fiber->getId();
    return 0;
}

Fiber::Fiber()
{
    // std::cout << "构造主协程" << std::endl;
    m_state = EXEC; // 第一个协程 执行中
    SetThis(this);

    if(getcontext(&m_ctx))  // 获取当前线程的上下文，并将其保存到协程的上下文中
    {
        SHUAI_ASSERT2(false, "getcontext");
    }

    ++s_fiber_count;
    SHUAI_LOG_DEBUG(g_logger) << "Fiber::Fiber";
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize)
    :m_id(++s_fiber_id)
    ,m_cb(cb)
{
    // std::cout << "构造子协程" << std::endl;
    ++s_fiber_count;
    m_stacksize =stacksize ? stacksize : g_fiber_stack_size->getValue();  // 若stacksize为0则将协程栈的大小设为1M

    // 栈的生成
    m_stack = StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx))                     // 调用 getcontext 获取当前执行上下文，并将其保存在 m_ctx 中
    {
        SHUAI_ASSERT2(false, "getcontext");    // getcontext失败
    }
    m_ctx.uc_link = nullptr;                   // 设置上下文链接为 nullptr，说明当前上下文结束后不会切换到其他上下文
    m_ctx.uc_stack.ss_sp = m_stack;            // 指定协程使用的栈空间
    m_ctx.uc_stack.ss_size = m_stacksize;      // 设置栈的大小

    makecontext(&m_ctx, &Fiber::MainFunc, 0);  // 设置协程执行的起始函数为 MainFunc。这是协程实际运行时调用的函数 MainFunc执行m_cb
    SHUAI_LOG_DEBUG(g_logger) << "Fiber::Fiber id = " << m_id;
}

Fiber::~Fiber()
{
    --s_fiber_count;
    if(m_stack)
    {
        // 确保协程的状态是 TERM（终止）、INIT（初始化）或 EXCEPT（异常）。这确保在销毁时协程处于合理的状态
        SHUAI_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
        StackAllocator::Dealloc(m_stack, m_stacksize);
    }
    else
    {
        // 主协程是没有栈的
        SHUAI_ASSERT(!m_cb);
        SHUAI_ASSERT(m_state == EXEC);

        Fiber* cur = t_fiber;
        if(cur == this)
        {
            SetThis(nullptr);
        }
    }
    SHUAI_LOG_DEBUG(g_logger) << "Fiber::~Fiber id = " << m_id;
}

// 协程执行完或者有问题时 此时内存未释放，基于这个内存，重新设置协程函数并重置状态，省了内存的分配和释放
void Fiber::reset(std::function<void()> cb)   
{
    SHUAI_ASSERT(m_stack);
    SHUAI_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);     // 只有在结束或者INIT状态才能重置
    m_cb = cb;
    if(getcontext(&m_ctx))
    {
        SHUAI_ASSERT2(false, "getcontext"); // getcontext失败
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT; 
}

// 自己开始执行，切换到当前协程执行  即正在操作的协程与正在运行的协程交换 resume
void Fiber::swapIn()                          
{
    SetThis(this);
    SHUAI_ASSERT(m_state != EXEC);

    // &(*t_threadFiber)->m_ctx：这是即将被切换出去的协程的上下文。它代表当前正在执行的协程的状态；&m_ctx：这是要切换到的协程的上下文，即当前协程的上下文
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx))   // 从主协程swap到当前协程   一旦上下文切换成功，目标协程就会开始执行它所绑定的函数
    {
        SHUAI_ASSERT2(false, "swapcontext");
    }
}

// 让出执行权  切换到后台 yield
void Fiber::swapOut()
{
    SetThis(t_threadFiber.get());

    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx))   // 从当前协程切换到主协程 t_threadFiber为主协程
    {
        SHUAI_ASSERT2(false, "swapcontext");
    }
}

// 设置当前协程  该函数到底是干什么的呢？ 将该协程设置为当前正在执行的协程
void Fiber::SetThis(Fiber* f)          
{
    t_fiber = f;  // 将协程f设置为当前正在执行的协程
}

// 获取当前正在执行的协程
Fiber::ptr Fiber::GetThis()
{
    if(t_fiber)
        return t_fiber->shared_from_this();
    // 下面为当前没有协程
    Fiber::ptr main_fiber(new Fiber);    // 默认构造 主协程
    SHUAI_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

// 协程切换到后台，并且设置为Ready状态  随时可以被调度执行，而不需要等待额外的条件
void Fiber::YieldToReady()            
{
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    cur->swapOut();
} 

// 协程切换到后台，并且设置为Hold状态  虽然它的上下文被保留，但并不意味着它随时可以被执行
// "保持状态"（Hold State）通常指的是协程在执行过程中暂时停止，但并未结束其生命周期。此时，协程的上下文仍然被保留，以便可以稍后恢复执行
void Fiber::YieldToHold()
{
    Fiber::ptr cur = GetThis();
    cur->m_state = HOLD;
    cur->swapOut();
}   

// 总协程数
uint64_t Fiber::TotalFibers()
{
    return s_fiber_count;
}         

void Fiber::MainFunc()
{
    Fiber::ptr cur = GetThis();   // GetThis()的shared_from_this()方法让引用计数加1
    SHUAI_ASSERT(cur);
    try
    {
        cur->m_cb();
        cur->m_cb = nullptr;  
        // 函数执行完后，协程的状态就为终止(TERM)了
        cur->m_state = TERM;
    }
    catch(const std::exception& e)
    {
        cur->m_state = EXCEPT;
        SHUAI_LOG_ERROR(g_logger) << "Fiber Except: " << e.what();
    }
    catch(...)
    {
        cur->m_state = EXCEPT;
        SHUAI_LOG_ERROR(g_logger) << "Fiber Except: ";
    }
    
    auto raw_ptr = cur.get();  // 手动让t_fiber的引用计数减1
    cur.reset();
    raw_ptr->swapOut();   // 将子协程切换回主协程
}

}