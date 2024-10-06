#pragma once 

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>

#include <vector>
#include <string>

namespace shuai
{

pid_t GetThreadId();
uint32_t GetFiberId();

void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);  // size为最大获得多少层，skip为越过前skip层
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

// 获取毫秒级/微秒级的时间
uint64_t GetCurrentMS();
uint64_t GetCurrentUS();

}