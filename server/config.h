#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <map>
#include <yaml-cpp/yaml.h>

#include "log.h"

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
        YAML::Node node;
        for(auto& i : v) node.push_back(YAML::Node(LexicalCast<T, std::string>()(i)));
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 序列化反序列化
// FromStr T operator()(const std::string&) 
// ToStr std::string operator()(const T&)
template <class T, class FromStr = LexicalCast<std::string, T>
                 , class ToStr = LexicalCast<T, std::string>>
class ConfigVar: public ConfigVarBase
{
public:
    typedef std::shared_ptr<ConfigVar> ptr;

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
            SHUAI_LOG_ERROR(SHUAI_LOG_ROOT()) << "ConfigVar::fromString exception" << e.what() << " convert: string to " << typeid(m_val).name();
        }
        return false;
    }

    const T getValue() const { return m_val; }
    void setValue(const T& v) { m_val = v; }
private:
    T m_val;
};

class Config
{
public:
    // 写了一个子类继承ConfigVarBase，是因为map中存放的必须为确定的数据类型，不能这样写typedef std::map<std::string, ConfigVar<T>> ConfigVarMap;
    // 这样写值处就可以存放子类和父类对象
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& default_value, const std::string& description = "")
    {
        auto tmp = Lookup<T>(name);
        if(tmp)
        {
            SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "Lookup name=" << name << " exists";
            return tmp;
        }   

        if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
        {
            SHUAI_LOG_ERROR(SHUAI_LOG_ROOT()) << "Lookup name invalid" << name; 
            throw std::invalid_argument(name); 
        }

        // 若是不特化 就是从这里有错误
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        // 父类的指针指向子类对象，会发生多态 
        s_datas[name] = v;
        return v;
    }
    
    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name)
    {
        auto it = s_datas.find(name);
        if(it == s_datas.end()) return nullptr;

        return std::dynamic_pointer_cast<ConfigVar<T>> (it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);

    static ConfigVarBase::ptr LookupBase(const std::string& name);
private:
    // 静态的只会存储这一个，所有的键值对都是存放在这里了
    static ConfigVarMap s_datas;
};

}
