#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <map>

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
    {}
    virtual ~ConfigVarBase() {}

    const std::string& getName() { return m_name; }
    const std::string& getDescription() { return m_description; }

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;

protected:
    std::string m_name;
    std::string m_description;
};

template <class T>
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
            boost::lexical_cast<std::string>(m_val);
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
            m_val = boost::lexical_cast<T>(val);
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
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& default_value, const std::string& description = "")
    {
        auto tmp = Lookup<T>(name);
        if(tmp)
        {
            SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "Lockup name=" << name << " exists";
            return tmp;
        }

        if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789") != std::string::npos)
        {
            SHUAI_LOG_ERROR(SHUAI_LOG_ROOT()) << "Lookup name invalid" << name; 
            throw std::invalid_argument(name); 
        }

        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
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
private:
    static ConfigVarMap s_datas;
};

}