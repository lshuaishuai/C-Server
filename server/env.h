#pragma once

#include <map>
#include <vector>
#include <utility>
#include <iomanip>
#include <iostream>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "singleton.h"
#include "thread.h"

namespace shuai
{

class Env
{
public:
    typedef RWMutex RWMutexType;
    bool init(int argc, char** argv);

    void add(const std::string& key, const std::string& val);
    bool has(const std::string& key);
    void del(const std::string& key);
    std::string get(const std::string& key, const std::string& default_value = "");

    void addHelp(const std::string& key, const std::string& desc);
    void removeHelp(const std::string& key);
    void printHelp();

    const std::string& getExe() const { return m_exe; }
    const std::string& getCwd() const { return m_cwd; }

    bool setEnv(const std::string& key, const std::string& val);
    std::string getEnv(const std::string& key, const std::string& default_value = "");

    std::string getAbsolutePath(const std::string& path) const;

private:
    RWMutexType m_mutex;
    std::map<std::string, std::string> m_args;
    std::vector<std::pair<std::string, std::string>> m_helps;  // 参数：其意义

    std::string m_program;
    std::string m_exe;
    std::string m_cwd;   // 可执行文件所在的路径为cwd
};

typedef shuai::Singleton<Env> EnvMgr;

}