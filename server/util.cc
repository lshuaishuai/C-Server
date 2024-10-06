#include "util.h"
#include "log.h"
#include "fiber.h"

#include <execinfo.h>

namespace shuai
{

shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system"); 

pid_t GetThreadId() { return syscall(SYS_gettid); }
uint32_t GetFiberId() { return shuai::Fiber::GetFiberId(); }

// size为最大获得多少层，skip为越过前skip层
// 用于获取当前线程的调用栈（backtrace），每一层都存储在vector中
void Backtrace(std::vector<std::string>& bt, int size, int skip)  
{
    // 用指针 避免在使用栈上太大的空间
    // array 是一个指向 void* 的指针数组，用于保存调用栈中的每个栈帧的地址。每个地址对应一个函数调用
    void** array = (void**)malloc((sizeof(void*) * size));
    // 这个函数会填充 array，将当前线程的栈帧信息写入到这个数组中。size 参数指定了要捕获的最大栈帧数量
    size_t s = ::backtrace(array, size);

    // 将array里面的这些地址转换为可读的符号信息（例如函数名、文件名和行号），并返回一个字符串数组
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

// 用于获取当前线程的调用栈（backtrace），并将其格式化为字符串
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

uint64_t GetCurrentMS()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

uint64_t GetCurrentUS()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}

}