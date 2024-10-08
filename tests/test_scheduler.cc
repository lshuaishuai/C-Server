#include "server/server.h"

shuai::Logger::ptr g_logger = SHUAI_LOG_ROOT();

void test_fiber()
{
    static int s_count = 5;
    SHUAI_LOG_INFO(g_logger) << "test in fiber s_count = " << s_count << " fiber id = " << shuai::GetFiberId();
    sleep(1);
    if(--s_count >= 0)
    {
        shuai::Scheduler::GetThis()->schedule(&test_fiber); // 将此函数设置到任务队列中去 schedule的第二个参数可以指定任务在哪个线程中执行
    }
}

int main(int argc, char** argv)
{
    SHUAI_LOG_INFO(g_logger) << "main";
    shuai::Scheduler sc(3, true, "test");
    sc.start(); 
    sleep(2);
    SHUAI_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber);  // 再这里提醒了一次
    sc.stop();
    SHUAI_LOG_INFO(g_logger) << "over";

    return 0;
}
