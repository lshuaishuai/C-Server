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
    server->start();
}

int main()
{
    shuai::IOManager iom(2);
    iom.schedule(run);
    return 0;
}