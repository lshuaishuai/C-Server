#include "server/http/http_server.h"
#include "server/server.h"

static shuai::Logger::ptr g_logger = SHUAI_LOG_ROOT();

void run()
{
    shuai::http::HttpServer::ptr server(new shuai::http::HttpServer);
    shuai::Address::ptr addr = shuai::Address::LookupAnyIPAddress("0.0.0.0:8080");
    while(!server->bind(addr))
    {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/server/xx", [](shuai::http::HttpRequest::ptr req, shuai::http::HttpResponse::ptr rsp, shuai::http::HttpSession::ptr session){
        rsp->setBody(req->toString());
        return 0;
    });
    
    sd->addGlobServlet("/server/*", [](shuai::http::HttpRequest::ptr req, shuai::http::HttpResponse::ptr rsp, shuai::http::HttpSession::ptr session){
        rsp->setBody("Glob:\r\n" + req->toString());
        return 0;
    });
    
    server->start();
}

int main()
{
    shuai::IOManager iom(2);
    iom.schedule(run);
    return 0;
}