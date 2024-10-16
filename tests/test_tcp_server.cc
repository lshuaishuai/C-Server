#include "server/tcp_server.h"
#include "server/server.h"

#include <vector>

static shuai::Logger::ptr g_logger = SHUAI_LOG_ROOT();

void run()
{
    auto addr = shuai::IPAddress::LookupAny("0.0.0.0:8033");
    // auto addr2 = shuai::UnixAddress::ptr(new shuai::UnixAddress("/tmp/unix_addr"));
    // SHUAI_LOG_INFO(g_logger) << *addr << " - " << *addr2;
    std::vector<shuai::Address::ptr> addrs;
    addrs.push_back(addr);
    // addrs.push_back(addr2);

    shuai::TcpServer::ptr tcp_server(new shuai::TcpServer);
    std::vector<shuai::Address::ptr> fialds;
    while(!tcp_server->bind(addrs, fialds))
    {
        sleep(2);
    }
    tcp_server->start();
}

int main(int argc, char** argv)
{
    shuai::IOManager iom(2);
    iom.schedule(run);
    return 0;
}