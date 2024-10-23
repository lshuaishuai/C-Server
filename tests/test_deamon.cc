#include "server/daemon.h"
#include "server/iomanager.h"
#include "server/log.h"

static shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system"); 

shuai::Timer::ptr timer;
int server_main(int argc, char** argv)
{
    SHUAI_LOG_INFO(g_logger) << shuai::ProcessInfoMgr::GetInstance()->toString();
    shuai::IOManager iom(1);
    timer = iom.addTimer(1000, []()
    {
        SHUAI_LOG_INFO(g_logger) << "onTimer";
        static int count = 0;
        if(++count > 10) timer->cancel();
    }, true);
    return 0;
}

int main(int argc, char** argv)
{
    return shuai::start_daemon(argc, argv, server_main, argc != 1);
}