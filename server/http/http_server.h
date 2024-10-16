#pragma once

#include "server/tcp_server.h"
#include "http_session.h"

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

protected:
    virtual void handleClient(Socket::ptr client) override;

private:
    bool m_isKeepalive;

};

}
}