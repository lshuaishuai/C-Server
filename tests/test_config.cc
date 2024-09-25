#include <iostream>
#include <vector>

#include "../server/log.h"
#include "../server/config.h"
#include <yaml-cpp/yaml.h>


// 还会将这里的数据放入到s_datas中
shuai::ConfigVar<int>::ptr g_int_value_config = 
    shuai::Config::Lookup("system.port", (int)8080, "system port");

shuai::ConfigVar<float>::ptr g_float_value_config = 
    shuai::Config::Lookup("system.value", (float)10.2f, "system value");

shuai::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config = 
    shuai::Config::Lookup("system.int_vec", std::vector<int>{1,2}, "system int vec");

void print_yaml(const YAML::Node& node, int level)
{

    if(node.IsScalar())
    {
        SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << std::string(level*4, ' ') << node.Scalar() << " - " << node.Type() << " - " << level;
    }
    else if(node.IsNull())
    {
        SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << std::string(level*4, ' ') << "NULL - " << node.Type() << " - " << level;
    }
    else if(node.IsMap())
    {
        for(auto it = node.begin(); it != node.end(); ++it)
        {
            SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << std::string(level*4, ' ') << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level+1);
        }
    }
    else if(node.IsSequence())
    {
        for(size_t i = 0; i < node.size(); ++i)
        {
            SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << std::string(level*4, ' ') << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level+1);
        }
    }
}

void test_yaml()
{
    // Node 是 yaml-cpp 中的核心概念，是最重要的数据结构，它用于存储解析后的 yaml 信息
    // Scalar（标量）可能被映射为 0 或其他数字。
    // Sequence（序列）可能被映射为 1 或其他数字。
    // Map（映射）可能被映射为 2 或其他数字。
    YAML::Node root = YAML::LoadFile("/home/shuaishuai/project/sylar_server/bin/conf/log.yml");
    print_yaml(root, 0);
}

void test_config()
{
    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "before: " << g_int_value_config->getValue(); 
    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "before: "  << g_float_value_config->toString();
    auto v = g_int_vec_value_config->getValue();
    for(auto& i : v)
    {
        SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "befor int_vec: " << i;
    }

    YAML::Node root = YAML::LoadFile("/home/shuaishuai/project/sylar_server/bin/conf/log.yml");
    shuai::Config::LoadFromYaml(root);

    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "after: " << g_int_value_config->getValue(); 
    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "after: "  << g_float_value_config->toString();
    
    v = g_int_vec_value_config->getValue();    
    for(auto& i : v)
    {
        SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "int_vec: " << i;
    }

}

int main(int argc, char** argv)
{
    test_config();

    // test_yaml();
    return 0; 
}