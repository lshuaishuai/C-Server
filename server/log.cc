#include "log.h"
#include <map>
#include <functional>

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
        os << logger->getName();
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

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time)
    :m_file(file)
    ,m_line(line)
    ,m_elapse(elapse)
    ,m_threadId(thread_id)
    ,m_fiberId(fiber_id)
    ,m_time(time)
    ,m_logger(logger)
    ,m_level(level)
{}

LogEventWrap::LogEventWrap(LogEvent::ptr e) 
    :m_event(e)
{}

LogEventWrap::~LogEventWrap()
{
    m_event->getLogger()->log(m_event->getLevel(), m_event);
    std::cout << "~LogEventWrap()" << std::endl;
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
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::addAppender(LogAppender::ptr appender)   
{
    // 外面新创建的LogAppender调用get、set
    if(!appender->getFormatter())
    {
        appender->setFormatter(m_formatter);  // Logger和Appender里面都有m_formater，这里不要搞混淆 将appender里面的设置为Logger里面的m_formater(已经初始化好了)
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender)
{
    for(auto it = m_appenders.begin(); it != m_appenders.end(); it++)
    {
        if(*it == appender)
        {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::log(LogLevel::Level level, const LogEvent::ptr event)
{
    if(level >= m_level)
    {
        auto self = shared_from_this();
        for(auto& i : m_appenders)
        {
            // 发生多态，这里是调用的子类重写的log 是Appender的log()
            i->log(self, level, event);
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
        std::cout << m_formatter->format(logger, level, event);
    }
}

FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename(filename)
{
    reopen();
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    if(level  >= m_level) m_filestream << m_formatter->format(logger, level, event);
}

bool FileLogAppender::reopen()
{
    if(m_filestream) m_filestream.close();

    m_filestream.open(m_filename);

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

// %xxx  %xxx{xxx}   %%
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
        XX(m, MessageFormatItem),
        XX(p, LevelFormatItem),
        XX(r, ElapseFormatItem),
        XX(c, NameFormatItem),
        XX(t, ThreadIdFormatItem),
        XX(n, NewLineFormatItem),
        XX(d, DateTimeFormatItem),
        XX(f, FilenameFormatItem),
        XX(l, LineFormatItem),
        XX(T, TabFormatItem),
        XX(F, FiberIdFormatItem),
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

    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
}
Logger::ptr LoggerManager::getLogger(const std::string& name)
{
    auto it = m_logger.find(name);
    return it == m_logger.end() ? m_root : it->second;
}

void LoggerManager::init()
{

}

}