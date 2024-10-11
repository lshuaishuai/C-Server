#include "server/socket.h"
#include "server/server.h"
#include "server/iomanager.h"

static shuai::Logger::ptr g_logger = SHUAI_LOG_ROOT();

void test_socket()
{
    shuai::IPAddress::ptr addr = shuai::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr)
    {
        SHUAI_LOG_INFO(g_logger) << "get address: " << addr->toString();
    }
    else
    {
        SHUAI_LOG_ERROR(g_logger) << "get address fail";
        return;
    }

    shuai::Socket::ptr sock = shuai::Socket::CreateTCP(addr);
    
    addr->setPort(80);
    if(!sock->connect(addr))
    {
        SHUAI_LOG_ERROR(g_logger) << "connect " << addr->toString() << " fail";
        return;
    }
    else SHUAI_LOG_INFO(g_logger) << "connect " << addr->toString() << " connected";

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0)
    {
        SHUAI_LOG_INFO(g_logger) << "send fail rt = " << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());
    if(rt <= 0)
    {
        SHUAI_LOG_INFO(g_logger) << "send fail rt = " << rt;
        return;
    }

    buffs.resize(rt);
    SHUAI_LOG_INFO(g_logger) << buffs; 
}

int main(int argc, char** argv)
{
    shuai::IOManager iom;
    iom.schedule(&test_socket);
    return 0;
}