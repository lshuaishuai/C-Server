#pragma once

#include <memory>
#include <functional>
#include <vector>

#include "iomanager.h"
#include "socket.h"
#include "address.h"
#include "noncopyable.h"

namespace shuai
{

class TcpServer: public std::enable_shared_from_this<TcpServer> , Noncopyable
{
public:
    typedef std::shared_ptr<TcpServer> ptr;

    TcpServer(shuai::IOManager* worker = shuai::IOManager::GetThis(), shuai::IOManager* acceptWorker = shuai::IOManager::GetThis());
    virtual ~TcpServer();

    virtual bool bind(shuai::Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& failds);
    virtual bool start();
    virtual void stop();

    uint64_t getRecvTimeout() const { return m_recvTimeout; }
    std::string getName() const { return m_name; }
    void setRecvTimeout(uint64_t v) { m_recvTimeout = v; }
    void setNmae(const std::string& v) { m_name = v; }

    bool isStop() const { return m_isStop; }

protected:
    virtual void handleClient(Socket::ptr client);
    virtual void startAccept(Socket::ptr sock);

private:
    std::vector<Socket::ptr> m_socks;  
    IOManager* m_worker;               // 工作线程池
    IOManager* m_acceptWorker;               // 工作线程池
    uint64_t m_recvTimeout;            // 
    std::string m_name;
    bool m_isStop;                     // server是否停止
};

}