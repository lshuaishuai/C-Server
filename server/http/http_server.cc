#include "http_server.h"
#include "server/log.h"

namespace shuai
{
namespace http
{
static shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system");

HttpServer::HttpServer(bool keepalive, shuai::IOManager* worker, shuai::IOManager* accept_worker)
    :TcpServer(worker, accept_worker)
    ,m_isKeepalive(keepalive)
{}

void HttpServer::handleClient(Socket::ptr client)
{
    shuai::http::HttpSession::ptr session(new HttpSession(client));
    do{
        auto req = session->recvRequest();
        if(!req)
        {
            SHUAI_LOG_WARN(g_logger) << "recv http request fail, errno = " << errno  
                                     << " errstr = " << strerror(errno)
                                     << " client: " << *client;
            break;
        }
        HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));
        rsp->setBody("hello sylar");

        SHUAI_LOG_INFO(g_logger) << "request: " << std::endl
                                 << *req;
        SHUAI_LOG_INFO(g_logger) << "response: " << std::endl
                                 << *rsp;

        session->sendResponse(rsp);
    }while(m_isKeepalive);
    session->close();
}
}
}