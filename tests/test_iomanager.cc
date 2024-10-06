#include "server/server.h"
#include "server/iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>

shuai::Logger::ptr g_logger = SHUAI_LOG_ROOT();

int sock = 0;

void test_fiber() {
    SHUAI_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    //sleep(3);

    //close(sock);
    //sylar::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "220.181.38.149", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        SHUAI_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        // epoll若是没有数据到达sock上，就已知不可读，但是是可写的
        shuai::IOManager::GetThis()->addEvent(sock, shuai::IOManager::READ, [](){
            SHUAI_LOG_INFO(g_logger) << "read callback";
        });   // 添加成功但是并没有发生相关的IO操作 
        shuai::IOManager::GetThis()->addEvent(sock, shuai::IOManager::WRITE, [](){
            SHUAI_LOG_INFO(g_logger) << "write callback";
            shuai::IOManager::GetThis()->cancelEvent(sock, shuai::IOManager::READ);  
            close(sock);
        });
    } else {
        SHUAI_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
            SHUAI_LOG_INFO(g_logger) << "add event end";

}

void test1() {
    std::cout << "EPOLLIN=" << EPOLLIN
              << " EPOLLOUT=" << EPOLLOUT << std::endl;
    shuai::IOManager iom(2, false);
    iom.schedule(&test_fiber);
}

shuai::Timer::ptr s_timer;
void test_timer()
{
    shuai::IOManager iom(2, false);
    s_timer = iom.addTimer(1000, [](){
        static int i = 0;
        SHUAI_LOG_INFO(g_logger) << "hello timer i = " << i;
        if(++i == 3) s_timer->cancel();
    }, true); 
    // SHUAI_LOG_DEBUG(g_logger) << "add Timer is success";
}

int main(int argc, char** argv) {
    // test1();
    test_timer();
    return 0;
}