#include "server/application.h"
#include "server/config.h"
#include "server/log.h"
#include "server/env.h"
#include "server/http/http_server.h"

namespace shuai
{
static shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system");

static shuai::ConfigVar<std::string>::ptr g_server_work_path = 
            shuai::Config::Lookup("server.work_path", std::string("/home/shuaishuai/apps/work/server"), "server work path");

static shuai::ConfigVar<std::string>::ptr g_server_pid_file = 
            shuai::Config::Lookup("server.pid_file", std::string("shuai.pid"), "server pid file");

struct HttpServerConf
{
    std::vector<std::string> address;
    int keepalive = 0;
    int timeout = 1000 * 2 * 60;
    std::string name;

    bool isValid() const { return !address.empty(); }
    bool operator==(const HttpServerConf& oth) const
    {
        return address == oth.address && keepalive == oth.keepalive && timeout == oth.timeout && name == oth.name;
    }
};

template<>
class LexicalCast<std::string, HttpServerConf>
{
public:
    HttpServerConf operator()(const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        HttpServerConf conf;
        conf.keepalive = node["keepalive"].as<int>(conf.keepalive);
        conf.timeout = node["timeout"].as<int>(conf.timeout);
        conf.name = node["name"].as<std::string>(conf.name);
        if(node["address"].IsDefined())
        {
            for(size_t i = 0; i < node["address"].size(); ++i)
            {
                conf.address.push_back(node["address"][i].as<std::string>());
            }
        }
        return conf;
    }
};

template<>
class LexicalCast<HttpServerConf, std::string>
{
public:
    std::string operator()(const HttpServerConf& conf)
    {
        YAML::Node node;
        node["name"] = conf.name;
        node["keepalive"] = conf.keepalive;
        node["timeout"] = conf.timeout;
        for(auto& i : conf.address)
        {
            node["address"].push_back(i);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

static shuai::ConfigVar<std::vector<HttpServerConf>>::ptr g_http_servers_conf = 
            shuai::Config::Lookup("http_servers", std::vector<HttpServerConf>(), "http server config");

Application* Application::s_instance = nullptr;

Application::Application()
{
    s_instance = this;
}

bool Application::init(int argc, char** argv)
{
    m_argc = argc;
    m_argv = argv;
    shuai::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    shuai::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    shuai::EnvMgr::GetInstance()->addHelp("c", "conf path default: ./conf");
    shuai::EnvMgr::GetInstance()->addHelp("p", "print help");
    
    if(!shuai::EnvMgr::GetInstance()->init(argc, argv))
    {
        shuai::EnvMgr::GetInstance()->printHelp();
        return false;
    }
    if(shuai::EnvMgr::GetInstance()->has("p"))
    {
        shuai::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    int run_type = 0;
    if(shuai::EnvMgr::GetInstance()->has("s")) {
        run_type = 1;
    }
    if(shuai::EnvMgr::GetInstance()->has("d")) {
        run_type = 2;
    }

    if(run_type == 0) {
        shuai::EnvMgr::GetInstance()->printHelp();
        return false;   
    }

    // 验证服务器是否启动
    std::string pidfile = g_server_work_path->getValue()
                                + "/" + g_server_pid_file->getValue();
    if(shuai::FSUtil::IsRunningPidfile(pidfile)) {
        SHUAI_LOG_ERROR(g_logger) << "server is running:" << pidfile;
        return false;
    }

    // 获取配置路径
    std::string conf_path = shuai::EnvMgr::GetInstance()->getAbsolutePath(shuai::EnvMgr::GetInstance()->get("c", "conf"));
    SHUAI_LOG_INFO(g_logger) << "load conf path:" << conf_path;
    // 加载配置
    shuai::Config::LoadFromConfDir(conf_path);

    // 创建工作路径
    if(!shuai::FSUtil::Mkdir(g_server_work_path->getValue()))
    {
        SHUAI_LOG_FATAL(g_logger) << "create work path [" << g_server_work_path->getValue() 
                                  << " errno = " << errno
                                  << " errstr = " << strerror(errno);
        return false;
    }

    // std::ofstream ofs(pidfile);
    // if(!ofs) {
    //     SHUAI_LOG_ERROR(g_logger) << "open pidfile " << pidfile << " failed";
    //     return false;
    // }
    // ofs << getpid();
    return true;
}

bool Application::run()
{
    bool is_daemon = shuai::EnvMgr::GetInstance()->has("d");
    return start_daemon(m_argc, m_argv,
            std::bind(&Application::main, this, std::placeholders::_1,
                std::placeholders::_2), is_daemon);
}

int Application::main(int argc, char** argv)
{
    std::string pidfile = g_server_work_path->getValue()
                                + "/" + g_server_pid_file->getValue();
    std::ofstream ofs(pidfile);
    if(!ofs) {
        SHUAI_LOG_ERROR(g_logger) << "open pidfile " << pidfile << " failed";
        return false;
    }
    ofs << getpid();

    shuai::IOManager iom(1);
    iom.schedule(std::bind(&Application::run_fiber, this));
    iom.stop();

    return 0;
}

bool Application::getServer(const std::string& type, std::vector<TcpServer::ptr>& svrs)
{
    return true;
}

void Application::listAllServer(std::map<std::string, std::vector<TcpServer::ptr> >& servers)
{

}

int Application::run_fiber()
{
    auto http_confs = g_http_servers_conf->getValue();
    for(auto& i : http_confs)
    {
        SHUAI_LOG_INFO(g_logger) << LexicalCast<HttpServerConf, std::string>()(i);
        std::vector<Address::ptr> address;
        for(auto& a : i.address)
        {
            size_t pos = a.find(":");
            if(pos == std::string::npos)
            {
                SHUAI_LOG_ERROR(g_logger) << "invalid address: " << a;
                continue;
            }
            auto addr = shuai::Address::LookupAny(a);
            if(addr)
            {
                address.push_back(addr);
                continue;
            }
            std::vector<std::pair<Address::ptr, uint32_t>> result;
            if(!shuai::Address::GetInterfaceAddresses(result, a.substr(0, pos)))
            {
                SHUAI_LOG_ERROR(g_logger) << "invalid address: " << a;
                continue;       
            }
            for(auto& x : result)
            {
                auto ipaddr = std::dynamic_pointer_cast<IPAddress>(x.first);
                if(ipaddr)
                {
                    ipaddr->setPort(atoi(a.substr(pos+1).c_str()));
                }
                address.push_back(ipaddr);
            }
        }
        shuai::http::HttpServer::ptr  server(new shuai::http::HttpServer(i.keepalive));
        std::vector<Address::ptr> fails;
        if(!server->bind(address, fails))
        {
            for(auto& x : fails)
            {
                SHUAI_LOG_ERROR(g_logger) << "bind address fail:" << *x;
            }
            exit(0);
        }
        if(!i.name.empty())
            server->setNmae(i.name);
        server->start();
        m_httpservers.push_back(server);
    }
    return 0;
}

}

