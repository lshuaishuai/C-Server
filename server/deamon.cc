#include "daemon.h"

namespace shuai
{

static shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system"); 
static shuai::ConfigVar<uint32_t>::ptr g_daemon_restart_interval
             = shuai::Config::Lookup("daemon.restart_interval", (uint32_t)5, "daemon restart interval");

std::string ProcessInfo::toString() const
{
    std::stringstream ss;
    ss << "[ProcessInfo parent_id = " << parent_id
       << " main_id = " << main_id
       << " parent_start_time = " << shuai::Timer2Str(parent_start_time)
       << " main_start_time = " << shuai::Timer2Str(main_start_time)
       << " restart_count = " << restart_count << "]";
    return ss.str();
}

static int real_start(int argc, char** argv, std::function<int(int argc, char** argv)> main_cb) { return main_cb(argc, argv); }

static int real_daemon(int argc, char** argv, std::function<int(int argc, char** argv)> main_cb)
{
    daemon(1, 0);
    ProcessInfoMgr::GetInstance()->parent_id = getpid();
    ProcessInfoMgr::GetInstance()->parent_start_time = time(0);
    while(true)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            // 子进程返回
            ProcessInfoMgr::GetInstance()->main_id = getpid();
            ProcessInfoMgr::GetInstance()->main_start_time = time(0);
            SHUAI_LOG_INFO(g_logger) << "process child pid = " << getpid();
            return real_start(argc, argv, main_cb);
        }
        else if(pid < 0)
        {
            SHUAI_LOG_ERROR(g_logger) << "fork fail return = " << pid
                                      << " errno = " << errno 
                                      << " errstr = " << strerror(errno);
            return -1;
        }
        else
        {
            // 父进程返回
            int status = 0;
            waitpid(pid, &status, 0);
            if(status)
            {    SHUAI_LOG_INFO(g_logger) << "child crash pid = " << pid << " status = " << status;}
            else 
            {
                SHUAI_LOG_INFO(g_logger) << "child dinished pid = " << pid;
                break;
            }
            ProcessInfoMgr::GetInstance()->restart_count += 1;
            sleep(g_daemon_restart_interval->getValue());
        }
    }
    return 0;
}

int start_daemon(int argc, char** argv, std::function<int(int i, char** c)> main_cb, bool is_daemon)
{
    if(!is_daemon) return real_start(argc, argv, main_cb);
    return real_daemon(argc, argv, main_cb);
}

}
