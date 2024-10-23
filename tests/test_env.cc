#include "server/env.h"
#include <iostream>
#include <fstream>

struct A
{
    A()
    {
        std::ifstream ifs("/proc/" + std::to_string(getpid()) + "/cmdline", std::ios::binary);
        std::string content;
        content.resize(4096);

        ifs.read(&content[0], content.size());
        content.resize(ifs.gcount());
        for(size_t i = 0; i < content.size(); ++i)
        {
            std::cout << i << " - " << content[i] << " - " << (int)content[i] << std::endl;
        }
    }
};

A a;

int main(int argc, char** argv)
{
    shuai::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    shuai::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    shuai::EnvMgr::GetInstance()->addHelp("p", "print help");

    if(!shuai::EnvMgr::GetInstance()->init(argc, argv))
    {
        shuai::EnvMgr::GetInstance()->printHelp();        
        return 0;
    }

    std::cout << "exe = " << shuai::EnvMgr::GetInstance()->getExe() << std::endl;
    std::cout << "cwd = " << shuai::EnvMgr::GetInstance()->getCwd() << std::endl;
    std::cout << "path = " << shuai::EnvMgr::GetInstance()->getEnv("PATH", "xxx") << std::endl;
    std::cout << "test = " << shuai::EnvMgr::GetInstance()->getEnv("TEST", "") << std::endl;
    std::cout << "set env " << shuai::EnvMgr::GetInstance()->setEnv("TEST", "yy") << std::endl;
    std::cout << "test = " << shuai::EnvMgr::GetInstance()->getEnv("TEST", "") << std::endl;


    if(shuai::EnvMgr::GetInstance()->has("p"))
    {
        shuai::EnvMgr::GetInstance()->printHelp();        
        return 0;
    }
    return 0;
}