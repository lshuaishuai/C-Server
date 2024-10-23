#pragma once

#include <functional>

#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "singleton.h"
#include "log.h"
#include "config.h"

namespace shuai
{

struct ProcessInfo
{
    // 父进程id
    pid_t parent_id = 0;
    // 主进程id -- 子进程
    pid_t main_id = 0;
    uint64_t parent_start_time = 0;
    uint64_t main_start_time = 0;
    // 主进程重启的次数
    uint32_t restart_count = 0;
    std::string toString() const;
};

typedef shuai::Singleton<ProcessInfo> ProcessInfoMgr;

// 启动程序可以选择用守护进程的方式，argc参数个数，argv参数值数组，main_cb启动函数，返回程序的执行结果
int start_daemon(int argc, char** argv, std::function<int(int i, char** c)> main_cb, bool is_daemon);

}