#include "server/server.h"

shuai::Logger::ptr g_logger = SHUAI_LOG_ROOT();

void run_in_fiber()
{
    SHUAI_LOG_INFO(g_logger) << "run_in_fiber begin";
    // shuai::Fiber::GetThis()->swapOut();
    shuai::Fiber::YieldToHold();  // 将当前协程设置为保持状态 跳到主协程
    SHUAI_LOG_INFO(g_logger) << "run_in_fiber end";
    shuai::Fiber::YieldToHold();
    // 这里子协程在执行完该函数后不能自动切换回主协程，我们在MainFunc中写一个swapOut让其切换带主协程
}

void test_fiber()
{
    SHUAI_LOG_INFO(g_logger) << "main begin -1";
    {
        shuai::Fiber::GetThis(); // 此时t_fiber为nullptr，会构造一个主协程
        // sleep(100);
        SHUAI_LOG_INFO(g_logger) << "main begin";
        shuai::Fiber::ptr fiber(new shuai::Fiber(run_in_fiber));  // 构造了子协程 并绑定所执行的函数
        // sleep(100);
        fiber->swapIn();  // 子协程执行  一旦上下文切换成功，目标协程就会开始执行它所绑定的函数
        SHUAI_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        SHUAI_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    SHUAI_LOG_INFO(g_logger) << "main after end2";
}

// 创建主协程 打印--> 创建子协程 --> 切换到子协程 --> 执行子协程的函数 --> 执行到一半，挂起子协程，主协程继续运行 
// --> 在切换到主协程 打印--> 切换到子协程 --> 子协程继续执行 --> 挂起 切换到主协程 打印 --> 切换到子协程，函数运行结束返回到主协程  程序结束
int main(int argc, char** argv)
{
    shuai::Thread::SetName("main");
    
    std::vector<shuai::Thread::ptr> thrs;
    for(int i = 0; i < 3; ++i)
    {
        thrs.push_back(shuai::Thread::ptr(new shuai::Thread(test_fiber, "name_" + std::to_string(i))));
    }
    for(auto i : thrs)
    {
        i->join();
    }
    return 0;
}