#include "thread.h"
#include "log.h"

namespace shuai
{
// thread_local 是一种存储类说明符，用于定义线程局部存储（Thread Local Storage, TLS）。当你声明一个变量为 thread_local，每个线程都会有该变量的独立实例，而不是共享同一个实例
static thread_local Thread* t_thread = nullptr;    // 指向当前的线程
static thread_local std::string t_thread_name = "UNKNOW";  // 由于使用了 thread_local 关键字，每个线程都有自己的独立副本

static shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system");

Semaphore::Semaphore(uint32_t count)
{
    if(sem_init(&m_semaphore, 0, count))
    {
        throw std::logic_error("sem_init error");
    }
}

Semaphore::~Semaphore()
{
    sem_destroy(&m_semaphore);
}

void Semaphore::wait()     // P操作
{
    if(sem_wait(&m_semaphore))
    {
        throw std::logic_error("sem_wait error");
    }
}

void Semaphore::notify()   // V操作
{
    if(sem_post(&m_semaphore))
    {
        throw std::logic_error("sem_post error");
    }
}

Thread* Thread::GetThis()             // 拿到自己当前线程，最一些相关的操作
{   
    return t_thread;
}

// 因此可以在没有线程对象实例的情况下调用。它用于获取当前线程的名称，而不需要引用特定的 Thread 实例
// 可以在任何地方直接获取当前线程的名称，便于日志记录或其他操作
const std::string& Thread::GetName()  // 给日志用的
{
    return t_thread_name;
}

void Thread::SetName(const std::string& name)
{
    if(t_thread) t_thread->m_name = name;
    t_thread_name = name;
}
 
Thread::Thread(std::function<void()> cb, const std::string& name)
    :m_cb(cb)
    ,m_name(name)
{
    if(name.empty()) m_name = "UNKNOW";
    // 成功返回0，失败返回错误码
    // run 函数的具体执行是在新线程启动后进行的，而不是在调用 pthread_create 时立即执行
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if(rt)
    {
        SHUAI_LOG_ERROR(g_logger) << "pthread_create thread fail, rt = " << rt << " name = " << name;
        throw std::logic_error("pthread_create error");
    }
    // 新线程在创建成功后执行绑定的函数
    m_semaphore.wait();
    
    // SHUAI_LOG_ERROR(g_logger) << "pthread_create thread success！";
}

void* Thread::run(void* arg)
{
    Thread* thread = (Thread*)arg;
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = shuai::GetThreadId();
    // 给线程命名
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

    std::function<void()> cb; // cb此时为空的状态
    cb.swap(thread->m_cb);  // 这样可以确保 m_cb 在调用后变为空或无效状态，防止在其他地方再次调用它

    thread->m_semaphore.notify();
    cb();
    return 0;
}

Thread::~Thread()
{
    if(m_thread)
        pthread_detach(m_thread);
}


void Thread::join()
{
    // 若线程是存在的
    if(m_thread) 
    {
        int rt = pthread_join(m_thread, nullptr);
        if(rt) 
        {
            SHUAI_LOG_ERROR(g_logger) << "pthread_join thread fail, rt = " << rt << " name = " << m_name;
            throw std::logic_error("pthread_join error");
        }
        m_thread = 0;
    }
}

}