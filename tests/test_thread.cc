#include "server/server.h"

#include <vector>
#include <unistd.h>

shuai::Logger::ptr g_logger = SHUAI_LOG_ROOT();
// shuai::RWMutex s_mutex;  // 创建互斥量

shuai::Mutex s_mutex;

int count = 0;

void fun1()
{
    SHUAI_LOG_INFO(g_logger) << "name: " << shuai::Thread::GetName()
                             << "  this.name: " << shuai::Thread::GetThis()->getName()
                             << "  id: " << shuai::GetThreadId()
                             << "  this.id: " << shuai::Thread::GetThis()->getId();

    for(int i = 0; i < 1000000; ++i) 
    {
        // shuai::RWMutex::WriteLock lock(s_mutex);
        // shuai::Mutex::Lock lock(s_mutex);
        ++count;
    }
}

void fun2()
{
    while(true)
    {
        SHUAI_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3()
{
    while(true)
    {
        SHUAI_LOG_INFO(g_logger) << "------------------------------------------------------";
    }
} 

int main(int argc, char** argv)
{
    SHUAI_LOG_INFO(g_logger) << "thread test start";
    YAML::Node root = YAML::LoadFile("/home/shuaishuai/project/sylar_server/bin/conf/log2.yml");
    shuai::Config::LoadFromYaml(root);
 
    std::vector<shuai::Thread::ptr> thrs; 
    for(int i = 0; i < 1; ++i)
    {
        shuai::Thread::ptr thr(new shuai::Thread(&fun2, "name_"+std::to_string(i*2)));
        shuai::Thread::ptr thr2(new shuai::Thread(&fun3, "name_"+std::to_string(i*2+1)));
        thrs.push_back(thr);
        thrs.push_back(thr2);
    }

    for(size_t i = 0; i < thrs.size(); ++i)
    {
        thrs[i]->join();
    }
    SHUAI_LOG_INFO(g_logger) << "thread test end";
    SHUAI_LOG_INFO(g_logger) << "count = " << count;



    return 0;
}