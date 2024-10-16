#include "tcp_server.h"
#include "config.h"
#include "log.h"

static shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system");

static shuai::ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout = 
       shuai::Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2), "tcp server read timeout");

namespace shuai
{
TcpServer::TcpServer(shuai::IOManager* worker, shuai::IOManager* acceptWorker)
    :m_worker(worker)
    ,m_acceptWorker(acceptWorker)
    ,m_recvTimeout(g_tcp_server_read_timeout->getValue())
    ,m_name("shuai/1.0.0")
    ,m_isStop(true)
{
    // SHUAI_LOG_INFO(g_logger) << "TcpServer::TcpServer(shuai::IOManager* worker, shuai::IOManager* acceptWorker)";

}

TcpServer::~TcpServer()
{
    for(auto& i : m_socks)
        i->close();

    m_socks.clear();
}

bool TcpServer::bind(shuai::Address::ptr addr)
{
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> failds;
    addrs.push_back(addr);
    return bind(addrs, failds);
}

bool TcpServer::bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& failds)
{
    for(auto& addr : addrs)
    {

        Socket::ptr sock = Socket::CreateTCP(addr);  // 创建好对应类型的socket
        if(!sock->bind(addr))
        {
            SHUAI_LOG_ERROR(g_logger) << "bind fail errno = " << errno
                                      << " errstr = " << strerror(errno)
                                      << " addr = [" << addr->toString() << "]";
            failds.push_back(addr);
            continue;
        }
        if(!sock->listen())
        {
            SHUAI_LOG_ERROR(g_logger) << "listen fail errno = " << errno
                                      << " errstr = " << strerror(errno)
                                      << " addr=[" << addr->toString() << "]";
            failds.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);
    }

    if(!failds.empty())
    {
        m_socks.clear();
        return false;
    }

    for(auto& i : m_socks)
        SHUAI_LOG_INFO(g_logger) << "server bind success: " << *i;

    return true;
}

void TcpServer::startAccept(Socket::ptr sock)
{
    while(!m_isStop)
    {
        Socket::ptr client = sock->accept();
        if(client)
        {
            client->setRecvTimeout(m_recvTimeout);
            m_worker->schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client));
        }
        else{
            SHUAI_LOG_ERROR(g_logger) << "accept errno = " << errno 
                                      << " errstr = " << strerror(errno);
        }
    }
}

bool TcpServer::start()
{
    if(!m_isStop) return true;
    m_isStop = false;

    for(auto& sock : m_socks)
    {
        m_acceptWorker->schedule(std::bind(&TcpServer::startAccept, shared_from_this(), sock));
    }
    return true;
}

void TcpServer::stop()
{
    m_isStop = true;
    auto self = shared_from_this();
    m_acceptWorker->schedule([self](){
        for(auto& sock : self->m_socks)
        {
            sock->cancelAll();
            sock->close();
        }
        self->m_socks.clear();
    });
}

void TcpServer::handleClient(Socket::ptr client)
{
    SHUAI_LOG_INFO(g_logger) << "handleClient: " << *client; 
}

}