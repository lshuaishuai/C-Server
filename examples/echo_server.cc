#include "server/tcp_server.h"
#include "server/log.h"
#include "server/iomanager.h"
#include "server/bytearray.h"

static shuai::Logger::ptr g_logger = SHUAI_LOG_ROOT();

class EchoServer: public shuai::TcpServer
{
public:
    EchoServer(int type);
    void handleClient(shuai::Socket::ptr client);

private:
    int m_type = 0;

};

EchoServer::EchoServer(int type)
    :m_type(type)
{}

void EchoServer::handleClient(shuai::Socket::ptr client)
{
    SHUAI_LOG_INFO(g_logger) << "handleClient " << *client;
    shuai::ByteArray::ptr ba(new shuai::ByteArray);
    while(true)
    {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        int rt = client->recv(&iovs[0], iovs.size());
        if(rt == 0)
        {    SHUAI_LOG_INFO(g_logger) << "client close: " << *client; break; }
        else if(rt < 0)
        {
            SHUAI_LOG_INFO(g_logger) << "client error rt = " << rt 
                                     << " error = " << errno
                                     << " errstr = " << strerror(errno);
            break;
        }
        ba->setPosition(ba->getPosition() + rt);

        ba->setPosition(0);
        if(m_type == 1)
        {    std::cout << ba->toString() << std::endl; }
        else
        {    std::cout << ba->toHexString() << std::endl; }
        std::cout.flush();
    }
}

int type = 1;

void run()
{
    EchoServer::ptr es(new EchoServer(type));
    auto addr = shuai::Address::LookupAny("0.0.0.0:8080");
    while(!es->bind(addr))
    {
       sleep(2); 
    }
    es->start();
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        SHUAI_LOG_INFO(g_logger) << "used as[" << argv[0] << "-t] or [" << argv[0] << " -b]";
        return 0;
    }

    if(!strcmp(argv[1], "-b")) type = 2;

    shuai::IOManager iom(2);
    iom.schedule(run);

    return 0;
}