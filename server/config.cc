#include "config.h"

namespace shuai
{
Config::ConfigVarMap Config::s_datas;

ConfigVarBase::ptr Config::LookupBase(const std::string& name)
{
    auto it = s_datas.find(name);
    return it == s_datas.end() ? nullptr : it->second;
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
        ConfigVarBase::ptr var = LookupBase(key);

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

}