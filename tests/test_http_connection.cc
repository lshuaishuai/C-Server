#include "server/http/http_connection.h"
#include "server/iomanager.h"
#include "server/server.h"

#include <iostream>

static shuai::Logger::ptr g_logger = SHUAI_LOG_ROOT();

void test_pool()
{
    shuai::http::HttpConnectionPool::ptr pool(new shuai::http::HttpConnectionPool("www.sylar.top", "", 80, 10, 1000*30, 5));

    shuai::IOManager::GetThis()->addTimer(1000, [pool](){
        auto r = pool->doGet("/", 300);
        SHUAI_LOG_INFO(g_logger) << r->toString();
    }, true);
}

void run()
{
    shuai::Address::ptr addr = shuai::Address::LookupAnyIPAddress("www.sylar.top:80");
    if(!addr)
    {
        SHUAI_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    shuai::Socket::ptr sock = shuai::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt)
    {
        SHUAI_LOG_INFO(g_logger) << "connect " << *addr << " failed";
        return;
    }

    shuai::http::HttpConnection::ptr conn(new shuai::http::HttpConnection(sock));
    shuai::http::HttpRequest::ptr req(new shuai::http::HttpRequest);
    req->setPath("/blog/");
    req->setHeader("host", "www.sylar.top");
    SHUAI_LOG_INFO(g_logger) << "req: " << std::endl << *req;
    
    conn->sendRequest(req);
    auto rsp = conn->recvResponse();

    if(!rsp)
    {
        SHUAI_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    SHUAI_LOG_INFO(g_logger) << "rsp: " << std::endl << *rsp;

    SHUAI_LOG_INFO(g_logger) << "--------------------------------------";

    auto ret = shuai::http::HttpConnection::DoGet("http://www.sylar.top/blog/", 300);
    SHUAI_LOG_INFO(g_logger) << "result = " << ret->result << " error = " << ret->error << " rsp = " << (ret->response ? ret->response->toString() : "");
    SHUAI_LOG_INFO(g_logger) << "--------------------------------------";
    test_pool();
}

int main(int argc, char** argv)
{
    shuai::IOManager iom(2);
    iom.schedule(run);
    return 0;
}