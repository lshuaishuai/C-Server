#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <yaml-cpp/yaml.h>
#include <string.h>
#include <sys/stat.h>

#include "log.h"
#include "thread.h"

namespace shuai
{

class ConfigVarBase  // 基类
{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& description)
        :m_name(name)
        ,m_description(description)
    {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }
    virtual ~ConfigVarBase() {}

    const std::string& getName() { return m_name; }
    const std::string& getDescription() { return m_description; }

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;
    virtual std::string getTypeName() const = 0;

protected:
    std::string m_name;
    std::string m_description;
};

// 定义通用的基础类型的解析
// F from_type T: to_type F到T的转换     若是不能转换为T的F类型，在编译期间就会出错
template <class F, class T>
class LexicalCast
{
public:
    T operator()(const F& v) { return boost::lexical_cast<T>(v); }
};

// 实现了vector的偏特化版本
template <class T>
class LexicalCast<std::string, std::vector<T>>
{
public:
    std::vector<T> operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);  // 将string类型转为YAML::Node类型
        typename std::vector<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i)
        {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::vector<T>, std::string>
{
public:
    std::string operator()(const std::vector<T>& v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 实现了list的偏特化版本
template <class T>
class LexicalCast<std::string, std::list<T>>
{
public:
    std::list<T> operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);  // 将string类型转为YAML::Node类型
        typename std::list<T> lst;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i)
        {
            ss.str("");
            ss << node[i];
            lst.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return lst;
    }
};

template <class T>
class LexicalCast<std::list<T>, std::string>
{
public:
    std::string operator()(const std::list<T>& v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));  // 调用了非特化版本
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 实现了set的偏特化版本
template <class T>
class LexicalCast<std::string, std::set<T>>
{
public:
    std::set<T> operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);  // 将string类型转为YAML::Node类型
        typename std::set<T> st;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i)
        {
            ss.str("");
            ss << node[i];
            st.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return st;
    }
};

template <class T>
class LexicalCast<std::set<T>, std::string>
{
public:
    std::string operator()(const std::set<T>& v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 实现了unordered_set的偏特化版本
template <class T>
class LexicalCast<std::string, std::unordered_set<T>>
{
public:
    std::unordered_set<T> operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);  // 将string类型转为YAML::Node类型
        typename std::unordered_set<T> ust;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i)
        {
            ss.str("");
            ss << node[i];
            ust.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return ust;
    }
};

template <class T>
class LexicalCast<std::unordered_set<T>, std::string>
{
public:
    std::string operator()(const std::unordered_set<T>& v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 实现了map的偏特化版本  这个版本只支持K为字符串类型的
template <class T>
class LexicalCast<std::string, std::map<std::string, T>>
{
public:
    std::map<std::string, T> operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);  // 将string类型转为YAML::Node类型
        typename std::map<std::string, T> mp;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end(); ++it)
        {
            ss.str("");
            ss << it->second;
            mp.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return mp;
    }
};

template <class T>
class LexicalCast<std::map<std::string, T>, std::string>
{
public:
    std::string operator()(const std::map<std::string, T>& v)
    {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v)
        {

            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 实现了unordered_map的偏特化版本  这个版本只支持K为字符串类型的
template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>>
{
public:
    std::unordered_map<std::string, T> operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);  // 将string类型转为YAML::Node类型
        typename std::unordered_map<std::string, T> ump;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end(); ++it)
        {
            ss.str("");
            ss << it->second;
            ump.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return ump;
    }
};

template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string>
{
public:
    std::string operator()(const std::unordered_map<std::string, T>& v)
    {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v)
        {

            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 序列化反序列化
// FromStr T operator()(const std::string&) 
// ToStr std::string operator()(const T&)
// 若是std::set<LogDefine> -> string的话，会去log.cc中找模板特化版本的LexicalCast
template <class T, class FromStr = LexicalCast<std::string, T>
                 , class ToStr = LexicalCast<T, std::string>>
class ConfigVar: public ConfigVarBase
{
public:
    typedef RWMutex RWMutexType;
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;

    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        :ConfigVarBase(name, description)
        ,m_val(default_value)
    {}

    // 将T类型转换为string类型
    virtual std::string toString() override
    {
        try
        {
            // return boost::lexical_cast<std::string>(m_val);
            RWMutex::ReadLock lock(m_mutex);
            return ToStr()(m_val);
        }
        catch(const std::exception& e)
        {
            SHUAI_LOG_ERROR(SHUAI_LOG_ROOT()) << "ConfigVar::toString exception" << e.what() << " convert: " << typeid(m_val).name() << " to string";
        }
        return "";
    }

    // 将string类型转换为T类型
    virtual bool fromString(const std::string& val) override
    {
        try
        {
            // m_val = boost::lexical_cast<T>(val);
            setValue(FromStr()(val));
        }
        catch(const std::exception& e)
        {
            SHUAI_LOG_ERROR(SHUAI_LOG_ROOT()) << "ConfigVar::fromString exception" << e.what() << " convert: string to " 
                                              << typeid(m_val).name() << " - " << val;
        }
        return false;
    }

    const T getValue() { 
        RWMutex::ReadLock lock(m_mutex);
        return m_val; 
    }

    void setValue(const T& v) 
    { 
        {
            RWMutex::ReadLock lock(m_mutex);
            if(v == m_val) return;
            for(auto& i : m_cbs)
            {
                i.second(m_val, v);  // 在这里执行回调，也就是函数对象on_change_cb  第18节课中T为std::set<LogDefine>类型
            }
        }
        RWMutex::WriteLock lock(m_mutex);
        m_val = v;
    }

    std::string getTypeName() const override { return typeid(m_val).name(); }

    // 事件更改机制 当一个配置项发生修改的时候，可以反向通知对应的代码
    uint64_t addListener(on_change_cb cb) { 
        static uint64_t s_fun_id = 0;
        RWMutex::WriteLock lock(m_mutex);
        ++s_fun_id;
        m_cbs[s_fun_id] = cb; 
        return s_fun_id;
    }

    void delListener(uint64_t key) { 
        RWMutex::WriteLock lock(m_mutex);
        m_cbs.erase(key); 
    }

    on_change_cb getListener(uint64_t key)
    {
        RWMutex::ReadLock lock(m_mutex);
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    void clearListener() { 
        RWMutex::WriteLock lock(m_mutex);
        m_cbs.clear(); 
    }
private:
    RWMutexType m_mutex;
    T m_val;
    // 变更回调函数组 key：要求唯一
    std::map<uint64_t, on_change_cb> m_cbs;
};

class Config
{
public:
    // 写了一个子类继承ConfigVarBase，是因为map中存放的必须为确定的数据类型，不能这样写typedef std::map<std::string, ConfigVar<T>> ConfigVarMap;
    // 这样写值处就可以存放子类和父类对象
    typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    typedef RWMutex RWMutexType;

    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& default_value, const std::string& description = "")
    {
        RWMutexType::WriteLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if(it != GetDatas().end())
        {
            // 如果找到，尝试将找到的 ConfigVarBase::ptr 类型转换为 ConfigVar<T>::ptr 类型（具体类型的智能指针）。
            // 如果转换成功，则返回该变量；如果失败，记录错误并返回 nullptr
            // 若it->second为ConfigVar<int> 将其转换为ConfigVar<float>可以转换成功吗？ -- 不可以
            // std::dynamic_pointer_cast 进行类型转换时，转换的成功与否基于运行时类型检查。
            // 该方法用于将一个智能指针的类型安全地转换成另一个类型，但仅当目标类型实际上是源类型的派生类时才能成功。这是多态的一个关键特征。
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if(tmp)
            {
                SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "Lookup name=" << name << " exists";
                return tmp;
            }
            else
            {
                SHUAI_LOG_ERROR(SHUAI_LOG_ROOT()) << "Lookup name=" << name << " exists but type not " << typeid(T).name() << " real_type=" << it->second->getTypeName() << " " << it->second->toString();
                return nullptr;
            }
        }

        if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
        {
            SHUAI_LOG_ERROR(SHUAI_LOG_ROOT()) << "Lookup name invalid" << name; 
            throw std::invalid_argument(name); 
        }

        // 若是不特化 就是从这里有错误
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        // 父类的指针指向子类对象，会发生多态 
        GetDatas()[name] = v;
        return v;
    }
    
    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name)
    {
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if(it == GetDatas().end()) return nullptr;

        return std::dynamic_pointer_cast<ConfigVar<T>> (it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);
    // 加载文件夹里面的配置文件
    static void LoadFromConfDir(const std::string& path);
    static ConfigVarBase::ptr LookupBase(const std::string& name);

    static void Visit(std::function<void(ConfigVarBase::ptr)> cb);
private:
    // 静态的只会存储这一个，所有的键值对都是存放在这里了
    static ConfigVarMap& GetDatas(){
        static ConfigVarMap s_datas;
        return s_datas;
    }

    static RWMutexType& GetMutex() {
        static RWMutexType s_mutex;
        return s_mutex;
    }
};

}
