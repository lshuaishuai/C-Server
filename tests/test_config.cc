#include <iostream>
#include <vector>
#include <unistd.h>

#include "../server/log.h"
#include "../server/config.h"
#include <yaml-cpp/yaml.h>


// 还会将这里的数据放入到s_datas中
shuai::ConfigVar<int>::ptr g_int_value_config = 
    shuai::Config::Lookup("system.port", (int)8080, "system port");

shuai::ConfigVar<float>::ptr g_int_valuex_config = 
    shuai::Config::Lookup("system.port", (float)8081, "system port");   // 这行代码是检测，如果以有了相同的key，我们给它报错

shuai::ConfigVar<float>::ptr g_float_value_config = 
    shuai::Config::Lookup("system.value", (float)10.2f, "system value");

shuai::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config = 
    shuai::Config::Lookup("system.int_vec", std::vector<int>{1,2}, "system int vec");

shuai::ConfigVar<std::list<int>>::ptr g_int_list_value_config = 
    shuai::Config::Lookup("system.int_list", std::list<int>{1,2}, "system int list");

shuai::ConfigVar<std::set<int>>::ptr g_int_set_value_config = 
    shuai::Config::Lookup("system.int_set", std::set<int>{1,2}, "system int set");

shuai::ConfigVar<std::unordered_set<int>>::ptr g_int_uset_value_config = 
    shuai::Config::Lookup("system.int_uset", std::unordered_set<int>{1,2}, "system int uset");

shuai::ConfigVar<std::map<std::string, int>>::ptr g_str_int_map_value_config = 
    shuai::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k", 2}}, "system str int map");
    
shuai::ConfigVar<std::unordered_map<std::string, int>>::ptr g_str_int_umap_value_config = 
    shuai::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"k", 2}}, "system str int umap");

    
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

#define XX(g_var, name, prefix) \
    {\
        auto v = g_var->getValue();\
        for(auto& i : v) {\
            SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << #prefix " " #name ": " << i;\
        }\
        SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << #prefix " " #name " yaml : " << g_var->toString(); \
    }

#define XX_M(g_var, name, prefix) \
    {\
        auto v = g_var->getValue();\
        for(auto& i : v) {\
            SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << #prefix " " #name ": {" << i.first << " - " << i.second << "}";\
        }\
        SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << #prefix " " #name " yaml : " << g_var->toString(); \
    }

    XX(g_int_vec_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_uset_value_config, int_uset, before);
    XX_M(g_str_int_map_value_config, str_int_map, before);
    XX_M(g_str_int_umap_value_config, str_int_umap, before);

    YAML::Node root = YAML::LoadFile("/home/shuaishuai/project/sylar_server/bin/conf/log.yml");
    shuai::Config::LoadFromYaml(root);

    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "after: " << g_int_value_config->getValue(); 
    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "after: "  << g_float_value_config->toString();
    
    XX(g_int_vec_value_config, int_vec, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_uset_value_config, int_uset, after);
    XX_M(g_str_int_map_value_config, str_int_map, after);
    XX_M(g_str_int_umap_value_config, str_int_umap, after);
}

class Person {
public:
    Person() {};
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name=" << m_name
           << " age=" << m_age
           << " sex=" << m_sex
           << "]";
        return ss.str();
    }

    bool operator==(const Person& oth) const {
        return m_name == oth.m_name
            && m_age == oth.m_age
            && m_sex == oth.m_sex;
    }
};

namespace shuai
{
template<>
class LexicalCast<std::string, Person> {
public:
    Person operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }
};

template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& p) {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
}

shuai::ConfigVar<Person>::ptr g_person = 
    shuai::Config::Lookup("class.person", Person(), "class person");

shuai::ConfigVar<std::map<std::string, Person>>::ptr g_person_map = 
    shuai::Config::Lookup("class.map", std::map<std::string, Person>(), "class map");

shuai::ConfigVar<std::map<std::string, std::vector<Person>>>::ptr g_person_vec_map = 
    shuai::Config::Lookup("class.vec_map", std::map<std::string, std::vector<Person>>(), "class map");

void test_class()
{
    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "before: "  << g_person->getValue().toString() << " -  " << g_person->toString();

#define XX_PM(g_var, prefix) \
    { \
        auto m = g_person_map->getValue(); \
        for(auto& i : m) \
        { \
            SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << prefix << ": " << i.first << " - " << i.second.toString(); \
        } \
        SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << prefix << ": size=" << m.size(); \
    } 

    g_person->addListener(10, [](const Person& old_value, const Person& new_value)
    {
        SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "old_value=" << old_value.toString() << "new_value=" << new_value.toString();
    });

    XX_PM(g_person_map, "class.map before");
    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "before: " << g_person_vec_map->toString();   // 一开始没有给初始化，输出为空的
    YAML::Node root = YAML::LoadFile("/home/shuaishuai/project/sylar_server/bin/conf/log.yml");
    shuai::Config::LoadFromYaml(root);

    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "after: "  << g_person->getValue().toString() << " -  " << g_person->toString();
    XX_PM(g_person_map, "class.map after");
    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "after: " << g_person_vec_map->toString();
}

void test_log()
{
    static shuai::Logger::ptr system_log = SHUAI_LOG_NAME("system");   // 这里会创建一个新的Logger日志器，name为system
    SHUAI_LOG_INFO(system_log) << "hello system" << std::endl;

    std::cout << shuai::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/home/shuaishuai/project/sylar_server/bin/conf/log.yml");
    shuai::Config::LoadFromYaml(root);
    std::cout << "=======================" << std::endl;
    // 这里的输出没有变化，那就是LoggerMgr中的m_loggers没有改变
    std::cout << shuai::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    SHUAI_LOG_INFO(system_log) << "hello system" << std::endl;  // 由于这里改了日志打印格式，那么我们这里打印的日志就是新的对应的格式
    system_log->setFormatter("%d - %m%n");
    SHUAI_LOG_INFO(system_log) << "hello system" << std::endl;  

}  

int main(int argc, char** argv)
{
    // test_config();
    // test_yaml();
    // test_class();
    // Logger(root)是最先创建的，在main函数之前就创建了 在发生错误输出日志时，生成的root日志器
    test_log();
    return 0; 
}