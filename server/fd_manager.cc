#include "fd_manager.h"
#include "hook.h"

namespace shuai
{

FdCtx::FdCtx(int fd)
    :m_isInit(false)
    ,m_isSocket(false)
    ,m_userNonblock(false)
    ,m_sysNonblock(false)
    ,m_isClosed(false)
    ,m_fd(fd)
    ,m_recvTimeout(-1)
    ,m_sendTimeout(-1)
{
    init();
}

FdCtx::~FdCtx()
{

}

bool FdCtx::init()
{
    if(m_isInit) return true;
    m_recvTimeout = -1;
    m_sendTimeout = -1;
    
    struct stat fd_stat;                               // 声明 stat 结构体以获取文件状态
    if(-1 == fstat(m_fd, &fd_stat))
    {
        m_isInit = false;
        m_isSocket = false;
    }
    else{
        m_isInit = true;
        m_isSocket = S_ISSOCK(fd_stat.st_mode);        // st_mode：文件类型和权限的位掩码。
    }

    if(m_isSocket)
    {
        int flags = fcntl(m_fd, F_GETFL, 0);           // 获取当前文件状态标志
        if(!(flags & O_NONBLOCK))                      // 如果不是非阻塞模式
        {
            fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);// 设置为非阻塞模式
        }
        m_sysNonblock = true;
    }
    else m_sysNonblock = false;

    m_userNonblock = false;
    m_isClosed = false;
    return m_isInit;
}

void FdCtx::setTimeout(int type, uint64_t v)
{
    if(type == SO_RCVTIMEO) m_recvTimeout = v;
    else m_sendTimeout = v;
}

uint64_t FdCtx::getTimeout(int type)
{
    if(type == SO_RCVTIMEO) return m_recvTimeout;
    return m_sendTimeout;
}

FdManager::FdManager()
{
    m_datas.resize(64);
}

// 取一个fd，若fd不存在且auto_create=true则新创建一个fd
FdCtx::ptr FdManager::get(int fd, bool auto_create)
{
    if(fd == -1) return nullptr;

    RWMutexType::ReadLock lock(m_rmutex);
    // std::cout << "m_datas.size() = " << m_datas.size() << " auto_create = " << auto_create << std::endl;
    if((int)m_datas.size() <= fd)
    {    if(auto_create == false) return nullptr;}
    else
    {    if(m_datas[fd] || !auto_create) return m_datas[fd];}
    lock.unlock();
        
    RWMutexType::WriteLock lock2(m_wmutex);
    FdCtx::ptr ctx(new FdCtx(fd));
    if(fd >= (int)m_datas.size()) m_datas.resize(fd * 1.5);

    m_datas[fd] = ctx;
    lock2.unlock();
    return ctx;
}

void FdManager::del(int fd)
{
    RWMutexType::WriteLock lock(m_wmutex);
    if((int)m_datas.size() <= fd) return;
    m_datas[fd].reset();
}

}