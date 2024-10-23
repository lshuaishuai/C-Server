#include "config.h"
#include "env.h"
#include "util.h"

namespace shuai
{
// Config::ConfigVarMap Config::s_datas;

static shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system");

ConfigVarBase::ptr Config::LookupBase(const std::string& name)
{
    RWMutexType::ReadLock lock(GetMutex());
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}


// "A.B", 10
// A:
//   B: 10
//   C: str

static void ListAllMember(const std::string& prefix, const YAML::Node& node, std::list<std::pair<std::string, const YAML::Node>>& output)
{
    if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)  // 若prefix中包含不在字符集中的字符返回该字符的位置，否则返回npos
    {
        SHUAI_LOG_ERROR(SHUAI_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;  // 到了这儿就是包含不在字符集中的字符
    }
    output.push_back(std::make_pair(prefix, node));
    if(node.IsMap())
    {
        for(auto it = node.begin(); it != node.end(); ++it)
        {
            // 将树结构拍平 将node中的键值对拆分成普通的键值对了
            ListAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(), it->second, output);       
        }
    }
}

void Config::LoadFromYaml(const YAML::Node& root)
{
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;
    ListAllMember("", root, all_nodes);

    for(auto& i : all_nodes)
    {
        std::string key = i.first;
        if(key.empty()) continue;

        std::transform(key.begin(), key.end(), key.begin(), ::tolower);  // 将字符串 key 中的所有字符转换为小写字母。
        ConfigVarBase::ptr var = LookupBase(key);   // 这里在第18节课测试的时候var不为空 var为ConfigVar<std::set<LogDefine>>类型的智能指针

        if(var)
        {
            if(i.second.IsScalar())
            {
                var->fromString(i.second.Scalar());
            }
            else{

                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());
            }
        }
    }
}

static std::map<std::string, uint64_t> s_file2modifytime;
static shuai::Mutex s_mutex;

void Config::LoadFromConfDir(const std::string& path)
{
    std::string absolute_path = shuai::EnvMgr::GetInstance()->getAbsolutePath(path);  // 绝对路径
    std::vector<std::string> files;
    FSUtil::ListAllFile(files, absolute_path, ".yml");
    
    for(auto& i : files)
    {
        struct stat st;
        lstat(i.c_str(), &st);
        {
            shuai::Mutex::Lock lock(s_mutex);
            if(s_file2modifytime[i] == (uint64_t)st.st_mtime) continue; // 若文件没有修改则不需要重新加载
            s_file2modifytime[i] = (uint64_t)st.st_mtime;
        }

        try
        {
            YAML::Node root = YAML::LoadFile(i);
            LoadFromYaml(root);
            SHUAI_LOG_ERROR(g_logger) << "LoadConfigFile file = " << i << " seccuss";
        }
        catch(const std::exception& e)
        {
            SHUAI_LOG_ERROR(g_logger) << "LoadConfigFile file = " << i << " failed";
            SHUAI_LOG_ERROR(g_logger) << "e = " << e.what();
        }
        
    }
}    

void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb)
{
    RWMutexType::ReadLock lock(GetMutex());
    ConfigVarMap& m = GetDatas();
    for(auto it = m.begin(); it != m.end(); ++it)
    {
        cb(it->second);
    }
    }
}
