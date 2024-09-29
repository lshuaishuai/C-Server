#include "util.h"
#include "log.h"

#include <execinfo.h>

namespace shuai
{

shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system"); 

pid_t GetThreadId() { return syscall(SYS_gettid); }
uint32_t GetFiberId() { return 0; }

// size为最大获得多少层，skip为越过前skip层
void Backtrace(std::vector<std::string>& bt, int size, int skip)  
{
    // 用指针 避免在使用栈上太大的空间
    void** array = (void**)malloc((sizeof(void*) * size));
    size_t s = ::backtrace(array, size);

    char** strings = backtrace_symbols(array, s);
    if(strings == nullptr)
    {
        SHUAI_LOG_ERROR(g_logger) << "backtrace_symbols error";
        free(array);
        return;
    }

    for(size_t i = skip; i < s; ++i)
    {
        bt.push_back(strings[i]);
    }
    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix)   
{
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for(size_t i = 0; i < bt.size(); ++i)
    {
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}       

}