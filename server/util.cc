#include "util.h"
#include "log.h"
#include "fiber.h"

#include <execinfo.h>

namespace shuai
{

static shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system"); 

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

std::string Timer2Str(time_t ts, const std::string& format)
{
    struct tm tm;
    localtime_r(&ts, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return buf;
}

void FSUtil::ListAllFile(std::vector<std::string>& files, const std::string& path, const std::string& subfix)
{
    if(access(path.c_str(), 0) != 0) return;  // 文件路径不存在
    
    DIR* dir = opendir(path.c_str());
    if(dir == nullptr) return;                // 打开失败
    struct dirent* dp = nullptr;
    while((dp = readdir(dir)) != nullptr)
    {
        if(dp->d_type == DT_DIR)              // 若为目录递归读取
        {
            if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) continue;
            ListAllFile(files, path + "/" + dp->d_name, subfix);
        } 
        else if(dp->d_type == DT_REG)         // 普通文件
        {
            std::string filename(dp->d_name);
            if(subfix.empty()) 
                files.push_back(path + "/" + filename);
            else
            {
                if(filename.size() < subfix.size()) continue;
                if(filename.substr(filename.length() - subfix.size()) == subfix)
                {
                    // SHUAI_LOG_DEBUG(g_logger) << "filename = " << path + "/" + filename;
                    files.push_back(path + "/" + filename);
                }
            }
        }
    }
    closedir(dir);
}

static int __lstat(const char* file, struct stat* st = nullptr) {
    struct stat lst;
    int ret = lstat(file, &lst);
    if(st) {
        *st = lst;
    }
    return ret;
}

bool FSUtil::IsRunningPidfile(const std::string& pidfile) {
    if(__lstat(pidfile.c_str()) != 0) {
        return false;
    }
    std::ifstream ifs(pidfile);
    std::string line;
    if(!ifs || !std::getline(ifs, line)) {
        return false;
    }
    if(line.empty()) {
        return false;
    }
    pid_t pid = atoi(line.c_str());
    if(pid <= 1) {
        return false;
    }
    if(kill(pid, 0) != 0) {
        return false;
    }
    return true;
}


static int __mkdir(const char* dirname) {
    if(access(dirname, F_OK) == 0) {
        return 0;
    }
    return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

bool FSUtil::Mkdir(const std::string& dirname)
{
    if(__lstat(dirname.c_str()) == 0) {
        return true;
    }
    char* path = strdup(dirname.c_str());
    char* ptr = strchr(path + 1, '/');
    do {
        for(; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/')) {
            *ptr = '\0';
            if(__mkdir(path) != 0) {
                break;
            }
        }
        if(ptr != nullptr) {
            break;
        } else if(__mkdir(path) != 0) {
            break;
        }
        free(path);
        return true;
    } while(0);
    free(path);
    return false;
}

}