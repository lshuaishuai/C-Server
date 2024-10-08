#pragma once

#include <thread>
#include <functional>
#include <memory>
#include <iostream>
#include <atomic>
#include <semaphore.h>
#include <pthread.h>

#include "noncopyable.h"

// pthread_xxx
// c++11: std::thread库

namespace shuai
{

class Semaphore : Noncopyable
{
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    void wait();     // P操作
    void notify();   // V操作

private:
    Semaphore(const Semaphore&) = delete;
    Semaphore(const Semaphore&&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

private:
    sem_t m_semaphore;
};

template <class T>
struct ScopedLockImpl
{
public:
    ScopedLockImpl(T& mutex)
        :m_mutex(mutex)
    {
        m_mutex.lock();
        m_locked = true;
    }

    ~ScopedLockImpl() { unlock(); }

    void lock()
    {
        if(!m_locked)
        {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock()
    {
        if(m_locked)
        {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked;
};

template <class T>
struct ReadScopedLockImpl
{
public:
    ReadScopedLockImpl(T& mutex)
        :m_mutex(mutex)
    {
        m_mutex.rdlock();
        m_locked = true;
    }

    ~ReadScopedLockImpl() { unlock(); }

    void lock()
    {
        if(!m_locked)
        {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock()
    {
        if(m_locked)
        {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked;
};

template <class T>
struct WriteScopedLockImpl
{
public:
    WriteScopedLockImpl(T& mutex)
        :m_mutex(mutex)
    {
        m_mutex.wrlock();
        m_locked = true;
    }

    ~WriteScopedLockImpl() { unlock(); }

    void lock()
    {
        if(!m_locked)
        {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock()
    {
        if(m_locked)
        {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked;
};

class Mutex : Noncopyable
{
public:
    typedef ScopedLockImpl<Mutex> Lock;
    
    Mutex() { pthread_mutex_init(&m_mutex, nullptr); } 
    ~Mutex() { pthread_mutex_destroy(&m_mutex); }
    void lock() { pthread_mutex_lock(&m_mutex); }
    void unlock() { pthread_mutex_unlock(&m_mutex); }

private:
    pthread_mutex_t m_mutex;
};

class NullMutex
{
public:
    typedef ScopedLockImpl<NullMutex> Lock;
    NullMutex() {}
    ~NullMutex() {}
    void lock() {}
    void unlock() {}
};

// 读写锁
// 该代码实现了一个高效的读写锁机制，允许多个线程同时进行读取，而在写入时则需要独占锁
class RWMutex : Noncopyable
{
public:
    // 当然除了这两种以外，也可以创建彼得锁
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;

    RWMutex() { pthread_rwlock_init(&m_lock, nullptr); }
    ~RWMutex() { pthread_rwlock_destroy(&m_lock); }
    
    void rdlock() { pthread_rwlock_rdlock(&m_lock); }
    void wrlock() { pthread_rwlock_wrlock(&m_lock); }
    void unlock() { pthread_rwlock_unlock(&m_lock); }

private:
    pthread_rwlock_t m_lock; 
};

class NullRWMutex : Noncopyable
{
public:
    typedef ReadScopedLockImpl<NullRWMutex> ReadLock;
    typedef WriteScopedLockImpl<NullRWMutex> WriteLock;

    NullRWMutex() {}
    ~NullRWMutex() {}
    
    void rdlock() {}
    void wrlock() {}
    void unrdlock() {}
};

class SpinLock : Noncopyable
{
public:
    typedef ScopedLockImpl<SpinLock> Lock;

    SpinLock() { pthread_spin_init(&m_mutex, 0); }
    ~SpinLock() { pthread_spin_destroy(&m_mutex); }
    void lock() { pthread_spin_lock(&m_mutex); }
    void unlock() { pthread_spin_unlock(&m_mutex); }

private:
    pthread_spinlock_t m_mutex; 
};

class CASLock : Noncopyable
{
public:
    typedef ScopedLockImpl<CASLock> Lock;

    CASLock() {
        m_mutex.clear();
    }
    ~CASLock() {}

    void lock() {
        while(std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acq_rel));
    }
    void unlock() {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_relaxed);
    }

private:   
    volatile std::atomic_flag m_mutex;
};

class Thread
{
public:
    typedef std::shared_ptr<Thread> ptr;
    
    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();

    pid_t getId() const { return m_id; }
    // 适用于需要引用具体线程实例的情况，比如在管理或操作某个线程时
    const std::string& getName() const { return m_name; }

    void join();

    static Thread* GetThis();             // 拿到自己当前线程，最一些相关的操作
    static const std::string& GetName();  // 给日志用的
    static void SetName(const std::string& name);

private:

    // 禁止拷贝
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;

    static void* run(void* arg);
private:
    pid_t m_id = -1;                     // 线程id
    pthread_t m_thread = 0;
    std::function<void()> m_cb;     // 线程执行的函数
    std::string m_name;             // 线程名字

    Semaphore m_semaphore;
};
}