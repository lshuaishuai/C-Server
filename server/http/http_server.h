#pragma once

#include "server/tcp_server.h"
#include "http_session.h"
#include "http_servlet.h"

#include <memory>

namespace shuai
{
namespace http
{

class HttpServer: public TcpServer
{
public:
    typedef std::shared_ptr<HttpServer> ptr;

    HttpServer(bool keepalive = false, shuai::IOManager* worker = shuai::IOManager::GetThis(), shuai::IOManager* accept_worker = shuai::IOManager::GetThis());
    
    ServletDispatch::ptr getServletDispatch() const { return m_dispatch; }
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v; }

protected:
    virtual void handleClient(Socket::ptr client) override;

private:
    bool m_isKeepalive;
    ServletDispatch::ptr m_dispatch;

};

}
}