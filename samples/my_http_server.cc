#include "server/http/http_server.h"
#include "server/log.h"

shuai::Logger::ptr g_logger = SHUAI_LOG_ROOT();
shuai::IOManager::ptr worker;
void run() {
    g_logger->setLevel(shuai::LogLevel::INFO);
    shuai::Address::ptr addr = shuai::Address::LookupAnyIPAddress("0.0.0.0:8080");
    if(!addr) {
        SHUAI_LOG_ERROR(g_logger) << "get address error";
        return;
    }

    shuai::http::HttpServer::ptr http_server(new shuai::http::HttpServer(true, worker.get()));
    //sylar::http::HttpServer::ptr http_server(new sylar::http::HttpServer(true));
    bool ssl = false;
    while(!http_server->bind(addr)) {
        SHUAI_LOG_ERROR(g_logger) << "bind " << *addr << " fail";
        sleep(1);
    }

    if(ssl) {
        //http_server->loadCertificates("/home/apps/soft/sylar/keys/server.crt", "/home/apps/soft/sylar/keys/server.key");
    }

    http_server->start();
}

int main(int argc, char** argv) {
    shuai::IOManager iom(1);
    worker.reset(new shuai::IOManager(4, false));
    iom.schedule(run);
    return 0;
}