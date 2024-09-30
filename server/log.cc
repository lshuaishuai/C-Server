#include "log.h"
#include <map>
#include <functional>
#include <map>
#include <time.h>

#include "config.h"

namespace shuai
{
const char* LogLevel::ToString(LogLevel::Level level)
{
    switch(level)
    {
#define XX(name) \
    case LogLevel::name: \
        return #name; \
        break;

    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default: 
        return "UNKNOW";
    }
    return "UNKNOW";
}

LogLevel::Level LogLevel::FromString(const std::string& str)
{
#define XX(level, v) \
    if(str == #v){ \
        return LogLevel::level; \
    }

    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);

    return LogLevel::UNKNOW;
#undef XX
}

void LogAppender::setFormatter(LogFormatter::ptr val) 
{ 
    MutexType::Lock lock(m_mutex);
    m_formatter = val;
    if(m_formatter) m_hasFormatter = true;
    else m_hasFormatter = false; 
}

LogFormatter::ptr LogAppender::getFormatter()
{
    MutexType::Lock lock(m_mutex);
    return m_formatter;
}

class MessageFormatItem: public LogFormatter::FormatItem
{
public:
    MessageFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getContent();
    }
};

class LevelFormatItem: public LogFormatter::FormatItem
{
public:
    LevelFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << LogLevel::ToString(level);
    }
};

class ElapseFormatItem: public LogFormatter::FormatItem
{
public:
    ElapseFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getElapse();
    }
};

class NameFormatItem: public LogFormatter::FormatItem
{
public:
    NameFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        // 只有这个用到了logger，但是为了实现多态都得有这个参数
        os << event->getLogger()->getName();
    } 
};

class ThreadIdFormatItem: public LogFormatter::FormatItem
{
public:
    ThreadIdFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getThreadId();
    }
};

class FiberIdFormatItem: public LogFormatter::FormatItem
{
public:
    FiberIdFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getFiberId();
    }
};

class ThreadNameFormatItem: public LogFormatter::FormatItem
{
public:
    ThreadNameFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getThreadName();
    }
};

class DateTimeFormatItem: public LogFormatter::FormatItem
{
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format)
    {
        if(m_format.empty()) m_format = "%Y-%m-%d %H:%M:%S";
    }
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

class FilenameFormatItem: public LogFormatter::FormatItem
{
public:
    FilenameFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getFile();
    }
};

class LineFormatItem: public LogFormatter::FormatItem
{
public:
    LineFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getLine();
    }
};

class NewLineFormatItem: public LogFormatter::FormatItem
{
public:
    NewLineFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << std::endl;
    }
};

class StringFormatItem: public LogFormatter::FormatItem
{
public:
    StringFormatItem(const std::string& str)
        :m_string(str)
    {}

    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << m_string;
    }
private:
    std::string m_string;
};

class TabFormatItem: public LogFormatter::FormatItem
{
public:
    TabFormatItem(const std::string& str = "") {}
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << "\t";
    }
private:
    std::string m_string;
};

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time, const std::string& thread_name)
    :m_file(file)
    ,m_line(line)
    ,m_elapse(elapse)
    ,m_threadId(thread_id)
    ,m_fiberId(fiber_id)
    ,m_time(time)
    ,m_threadName(thread_name)
    ,m_logger(logger)
    ,m_level(level)
{}

LogEventWrap::LogEventWrap(LogEvent::ptr e) 
    :m_event(e)
{}

LogEventWrap::~LogEventWrap()
{
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}

void LogEvent::format(const char* fmt, ...)
{
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list al)
{
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if(len != -1)
    {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

std::stringstream& LogEventWrap::getSS()
{
    return m_event->getSS();
}


Logger::Logger(const std::string& name)
    :m_name(name)
    ,m_level(LogLevel::DEBUG)
{
    // 构造函数中不会为日志器生成默认的appender
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::setFormatter(LogFormatter::ptr val) 
{ 
    MutexType::Lock lock(m_mutex);
    m_formatter = val; 
    for(auto& i : m_appenders)
    {
        MutexType::Lock ll(i->m_mutex);
        if(!i->m_hasFormatter) i->m_formatter = m_formatter;
    }
}

void Logger::setFormatter(const std::string& val) 
{ 
    shuai::LogFormatter::ptr new_val(new shuai::LogFormatter(val));
    if(new_val->isError())
    {
        std::cout << "Logger setFormatter name=" << m_name <<" value=" << " invalid formatter" << std::endl;
        return;
    }
    // m_formatter.reset(new shuai::LogFormatter(val));
    setFormatter(new_val); 
}

std::string Logger::toYamlString()
{
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["name"] = m_name;
    if(m_level != LogLevel::UNKNOW)
        node["level"] = LogLevel::ToString(m_level);
    if(m_formatter)
        node["formatter"] = m_formatter->getPattern();

    for(auto& i : m_appenders)
    {
        node["appenders"].push_back(YAML::Load(i->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

LogFormatter::ptr Logger::getFormatter() 
{
    MutexType::Lock lock(m_mutex);
    return m_formatter; 
}

void Logger::addAppender(LogAppender::ptr appender)   
{
    MutexType::Lock lock(m_mutex);
    // 外面新创建的LogAppender调用get、set
    if(!appender->getFormatter())
    {
        MutexType::Lock ll(appender->m_mutex);
        appender->m_formatter = m_formatter;  // Logger和Appender里面都有m_formater，这里不要搞混淆 将appender里面的设置为Logger里面的m_formater(已经初始化好了)
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender)
{
    MutexType::Lock lock(m_mutex);
    for(auto it = m_appenders.begin(); it != m_appenders.end(); it++)
    {
        if(*it == appender)
        {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders()
{
    MutexType::Lock lock(m_mutex);
    m_appenders.clear();
}

void Logger::log(LogLevel::Level level, const LogEvent::ptr event)
{
    if(level >= m_level)
    {
        auto self = shared_from_this();
        // MutexType::Lock lock(m_mutex);   // 关掉这个锁以后性能会好一点
        if(!m_appenders.empty())
        {
            for(auto& i : m_appenders)
            {
                // 发生多态，这里是调用的子类重写的log 是Appender的log()
                i->log(self, level, event);
            }
        }
        else if(m_root)
        {
            m_root->log(level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr event)
{
    log(LogLevel::DEBUG, event);
}

void Logger::info(LogEvent::ptr event)
{
    log(LogLevel::INFO, event);

}
void Logger::warn(LogEvent::ptr event)
{
    log(LogLevel::WARN, event);

}

void Logger::error(LogEvent::ptr event)
{
    log(LogLevel::ERROR, event);

}

void Logger::fatal(LogEvent::ptr event)
{
    log(LogLevel::FATAL, event);

}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    if(level >= m_level)
    {
        MutexType::Lock lock(m_mutex);
        std::cout << m_formatter->format(logger, level, event);
    }
}

std::string StdoutLogAppender::toYamlString()
{
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    if(m_level != LogLevel::UNKNOW)
        node["level"] = LogLevel::ToString(m_level); 
    if(m_formatter && m_hasFormatter) node["formatter"] = m_formatter->getPattern();

    std::stringstream ss;
    ss << node;
    return ss.str();
}

FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename(filename)
{
    reopen();
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    if(level >= m_level) 
    {
        uint64_t now = time(0);
        // 若不作此reopen的操作，在删除掉日志的输出文件后，系统是不会感知到的，那么为了保证日志数据的正确保存，每隔一段时间来重新打开一次日志文件
        if(now != m_lastTime)   
        {
            reopen();
            m_lastTime = now;
        }

        MutexType::Lock lock(m_mutex);
        m_filestream << m_formatter->format(logger, level, event);
    }
}

std::string FileLogAppender::toYamlString()
{
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["file"] = m_filename;
    if(m_level != LogLevel::UNKNOW)
        node["level"] = LogLevel::ToString(m_level);
    if(m_formatter && m_hasFormatter)
    {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

bool FileLogAppender::reopen()
{
    MutexType::Lock lock(m_mutex);
    if(m_filestream) m_filestream.close();

    m_filestream.open(m_filename, std::ios::app);

    // 非0值转化为1，0值还是0
    return !!m_filestream;
}

LogFormatter::LogFormatter(const std::string& pattern)
    :m_pattern(pattern)
{
    init();   // 解析字符串并在这个函数中绑定每个%字母对应的子类
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    std::stringstream ss;

    // 此时流ss里面什么都没有呢
    for(auto& i : m_items)
    {
        // 这里也是发生了多态 FormatItem* 指向子类
        // 若是正常的字符串则正常打印 否则调用对应的函数
        // 将所有的输出输出到ss流中，再返回统一打印
        i->format(ss, logger, level, event);
    }
    // 将流中的字符串返回
    return ss.str();
}   

// %xxx  %xxx{xxx}   %%  解析格式
void LogFormatter::init()
{
    // str, format, type
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    // size_t last_pos = 0;
    for(size_t i = 0; i < m_pattern.size(); ++i)
    {
        if(m_pattern[i] != '%')
        {
            nstr.append(1, m_pattern[i]);
            continue;
        }
        
        // 走到这说明m_pattern[i]='%'   
        // 没有执行到m_pattern的尾部时
        if((i + 1) < m_pattern.size())
        {
            // 遇到连续的%，将第1个视为普通字符
            if(m_pattern[i+1] == '%')
            {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while(n < m_pattern.size())
        {
            // n位置不是字母且不是大括号的话就结束
            if(!fmt_status && (!isalpha(m_pattern[n])) && m_pattern[n] != '{' && m_pattern[n] != '}') 
            {
                str = m_pattern.substr(i+1, n-i-1);
                break;
            }
            if(fmt_status == 0) 
            {
                if(m_pattern[n] == '{')
                {
                    str = m_pattern.substr(i+1, n-i-1);
                    // std::cout << "*" << str << std::endl;
                    fmt_status = 1;  // 解析格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            }
            else if(fmt_status == 1)
            {
                if(m_pattern[n] == '}')
                {
                    fmt = m_pattern.substr(fmt_begin+1, n-fmt_begin-1);
                    // std::cout << "#" << str << std::endl;
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if(n == m_pattern.size())
            {
                if(str.empty()) str = m_pattern.substr(i+1);
            }
        }
        if(fmt_status == 0) 
        {
            if(!nstr.empty()) // 这一段是将两个%中间的内容添加到vector  完了以后继续将%对应的类型写到容器中
            {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        }
        else if(fmt_status == 1) 
        {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            m_error = true;
        }
        else if(fmt_status == 2)
        {
            if(!nstr.empty())
            {
                vec.push_back(std::make_tuple(nstr, "", 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n;
        }  
    }
    if(!nstr.empty())
    {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    // s_format_items 中存放的是字符串与对应格式项构造函数的映射关系。每个键是格式字符串（如 "m", "p" 等），对应的值是一个 lambda 表达式，该表达式接受一个字符串参数并返回一个 FormatItem 类型的智能指针（ptr）
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> s_format_items = {
#define XX(str, C) \
        {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt)); }} // #str 实现，这是字符串化操作符，将宏参数转换为字符串
        // 如果直接存储 FormatItem::ptr(new C(fmt)) 形式的结果，那么你就必须在填充 map 时立即创建所有这些对象，
        // 这意味着你需要在代码中已经具备所有必要的 fmt 字符串参数。这是不实际的，因为在定义 map 时，通常我们还不知道将会用什么具体的格式字符串来初始化这些对象。
        // 只有在通过特定的键访问 map 并调用相应的函数对象时，这些 lambda 函数才会执行
        XX(m, MessageFormatItem),         // m: 消息 
        XX(p, LevelFormatItem),           // p: 日志级别
        XX(r, ElapseFormatItem),          // r: 累计毫秒数
        XX(c, NameFormatItem),            // c: 日志名称
        XX(t, ThreadIdFormatItem),        // t: 线程id
        XX(n, NewLineFormatItem),         // n: 换行
        XX(d, DateTimeFormatItem),        // d: 时间
        XX(f, FilenameFormatItem),        // f: 文件名
        XX(l, LineFormatItem),            // l: 行号
        XX(T, TabFormatItem),             // T: Tab
        XX(F, FiberIdFormatItem),         // F: 协程id
        XX(N, ThreadNameFormatItem),      // N: 线程名称
#undef XX
    };
    for(auto& i : vec)
    {
        // 如果是那几个关键字母的话，三元组的第三个就是1，其余为0
        // 下面的FormatItem对象的构造都是构成了多态 父类指针指向子类对象
        if(std::get<2>(i) == 0)  //std::get<2>(i) 返回元组中第三个元素（索引从 0 开始）
        {
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        }
        else{
            auto it = s_format_items.find(std::get<0>(i));
            if(it == s_format_items.end())
            {
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                m_error = true;
            }
            else{
                // 将类对象放到map的值处
                m_items.push_back(it->second(std::get<1>(i)));   // it->second(std::get<1>(i))得到一个新的对象实例
            }
        }
        // std::cout << "{" << std::get<0>(i) << "} - {" << std::get<1>(i) << "} - {" << std::get<2>(i) << "}" << std::endl;
    }

}

LoggerManager::LoggerManager()
{
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));  // 若创建单例，会默认给StdLogAppender的日志输出地
    m_loggers[m_root->m_name] = m_root;

    init();
}

Logger::ptr LoggerManager::getLogger(const std::string& name)
{
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if(it != m_loggers.end())
        return it->second;
    Logger::ptr logger(new Logger(name));
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}

struct LogAppenderDefine
{   
    int type = 0;   // 1.File 2.stdout
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::string file;

    bool operator==(const LogAppenderDefine& oth) const
    {
        return type == oth.type
            && level == oth.level
            && formatter == oth.formatter
            && file == oth.file;
    }

};

struct LogDefine
{
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;

    bool operator==(const LogDefine& oth) const{
        return name == oth.name
            && level == oth.level
            && formatter == oth.formatter
            && appenders == oth.appenders;
    }

    bool operator<(const LogDefine& oth) const 
    {
        return name < oth.name;
    }
};

template<>
class LexicalCast<std::string, std::set<LogDefine>>
{
public:
    std::set<LogDefine> operator()(const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        std::set<LogDefine> st;
        for(size_t i = 0; i < node.size(); ++i)
        {
            auto n = node[i];
            if(!n["name"].IsDefined())
            {
                std::cout << "log config error: name is null, " << n << std::endl;
                continue;
            }
            LogDefine ld;   // 每个日志对应一个该对象
            ld.name = n["name"].as<std::string>();
            ld.level = LogLevel::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
            if(n["formatter"].IsDefined())
                ld.formatter = n["formatter"].as<std::string>();

            if(n["appenders"].IsDefined())
            {
                for(size_t x = 0; x < n["appenders"].size(); ++x)
                {
                    auto a = n["appenders"][x];
                    if(!a["type"].IsDefined())
                    {
                        std::cout << "log config error: appender type is null, " << a << std::endl;
                        continue;
                    }
                    std::string type = a["type"].as<std::string>();
                    LogAppenderDefine lad;  // 一个日志输出地对应一个该对象
                    if(type == "FileLogAppender")
                    {
                        lad.type = 1;
                        if(!a["file"].IsDefined())
                        {
                            std::cout << "log config error: file is null, " << a << std::endl;
                            continue;
                        }
                        lad.file = a["file"].as<std::string>();
                        if(a["formatter"].IsDefined())
                            lad.formatter = a["formatter"].as<std::string>();
                    }
                    else if(type == "StdoutLogAppender")
                    {
                        lad.type = 2;
                        if(a["formatter"].IsDefined()) lad.formatter = a["formatter"].as<std::string>();
                    }
                    else
                    {
                        std::cout << "log config error: appender type is invalid, " << a << std::endl;
                        continue;
                    }
                    ld.appenders.push_back(lad); // 
                }
            }
            st.insert(ld);  
        }
        return st;  // 在20课之前的log.yml中得到的set，size为2
    }
};

template <>
class LexicalCast<std::set<LogDefine>, std::string>
{
public:
    std::string operator()(const std::set<LogDefine>& v)
    {
        YAML::Node node;
        for(auto& i : v)
        {
            YAML::Node n;
            n["name"] = i.name;
            if(i.level != LogLevel::UNKNOW)
                n["level"] = LogLevel::ToString(i.level);

            if(i.formatter.empty()) n["formatter"] = i.formatter;

            for(auto& a : i.appenders)
            {
                YAML::Node na;
                if(a.type == 1)
                {
                    na["type"] = "FileLogAppender";
                    na["file"] = a.file;
                }
                else if(a.type == 2)
                {
                    na["type"] = "StdoutLogAppender";
                }
                if(a.level != LogLevel::UNKNOW)
                    na["level"] = LogLevel::ToString(a.level);
                if(!a.formatter.empty())
                {
                    na["formatter"] = a.formatter;
                }
                n["appenders"].push_back(na);
            }
            node.push_back(n);
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 用这个将config和log联系上了
shuai::ConfigVar<std::set<LogDefine>>::ptr g_log_defines = 
    shuai::Config::Lookup("logs", std::set<LogDefine>(), "logs config");  // std:set<LogDefine>()这是创建的old_value

struct LogIniter
{
    LogIniter(){
        // 将该函数放到ConfigVar::m_cbs的second中
        g_log_defines->addListener([](const std::set<LogDefine>& old_value, const std::set<LogDefine>& new_value){
            SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << "on_logger_conf_changed";
            for(auto& i : new_value)
            {
                auto it = old_value.find(i);
                shuai::Logger::ptr logger; // 不管是修改还是新增都是这个logger变量
                if(it == old_value.end())
                    // 新增logger
                    // logger.reset(new shuai::Logger(i.name));  // 第18节课中都为新增的log
                    logger = SHUAI_LOG_NAME(i.name);
                else
                {
                    if(!(i == *it))
                        /// 修改的logger
                        logger = SHUAI_LOG_NAME(i.name);
                }
                // 无论是修改还是新增，都需要将logger中的旧值换成新值
                logger->setLevel(i.level);
                if(!i.formatter.empty()) logger->setFormatter(i.formatter);

                logger->clearAppenders();
                for(auto& a : i.appenders)
                {
                    shuai::LogAppender::ptr ap;
                    if(a.type == 1) ap.reset(new FileLogAppender(a.file));
                    else if(a.type == 2) ap.reset(new StdoutLogAppender);
                    ap->setLevel(a.level);
                    if(!a.formatter.empty())
                    {
                        LogFormatter::ptr fmt(new LogFormatter(a.formatter));
                        if(!fmt->isError()) ap->setFormatter(fmt);
                        else std::cout << "log.name=" << i.name
                                       << " appender name=" << a.type 
                                       << " formatter=" << a.formatter 
                                       << " is invalid" << std::endl;
                    }
                    logger->addAppender(ap);
                }
            }
            // 删除
            for(auto& i : old_value)
            {
                auto it = new_value.find(i);
                if(it == new_value.end())
                {
                    // 删除logger -- 不是真正的删除，而是将日志级别设很高的值
                    auto logger = SHUAI_LOG_NAME(i.name);
                    logger->setLevel((LogLevel::Level)100);
                    logger->clearAppenders();
                }
            }
        });
    }
};

static LogIniter __log_init;

std::string LoggerManager::toYamlString()
{
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for(auto& i : m_loggers)
    {
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

void LoggerManager::init()
{

}

}   