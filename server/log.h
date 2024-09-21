#pragma once

#include <string>
#include <string.h>
#include <stdint.h>
#include <memory>
#include <iostream>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <time.h>
#include <stdarg.h>

#include "util.h"
#include "singleton.h"

#define SHUAI_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        shuai::LogEventWrap(shuai::LogEvent::ptr(new shuai::LogEvent(logger, level, __FILE__, __LINE__, 0, shuai::GetThreadId(), \
            shuai::GetFiberId(), time(0)))).getSS()    // 这里是返回LogEvent中的流，

#define SHUAI_LOG_DEBUG(logger) SHUAI_LOG_LEVEL(logger, shuai::LogLevel::DEBUG)
#define SHUAI_LOG_INFO(logger) SHUAI_LOG_LEVEL(logger, shuai::LogLevel::INFO)
#define SHUAI_LOG_WARN(logger) SHUAI_LOG_LEVEL(logger, shuai::LogLevel::WARN)
#define SHUAI_LOG_ERROR(logger) SHUAI_LOG_LEVEL(logger, shuai::LogLevel::ERROR)
#define SHUAI_LOG_FATAL(logger) SHUAI_LOG_LEVEL(logger, shuai::LogLevel::FATAL)

#define SHUAI_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        shuai::LogEventWrap(shuai::LogEvent::ptr(new shuai::LogEvent(logger, level, __FILE__, __LINE__, 0, shuai::GetThreadId(), \
            shuai::GetFiberId(), time(0)))).getEvent()->format(fmt, __VA_ARGS__)

#define SHUAI_LOG_FMT_DEBUG(logger, fmt, ...) SHUAI_LOG_FMT_LEVEL(logger, shuai::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SHUAI_LOG_FMT_INFO(logger, fmt, ...) SHUAI_LOG_FMT_LEVEL(logger, shuai::LogLevel::INFO, fmt, __VA_ARGS__)
#define SHUAI_LOG_FMT_WARN(logger, fmt, ...) SHUAI_LOG_FMT_LEVEL(logger, shuai::LogLevel::WARN, fmt, __VA_ARGS__)
#define SHUAI_LOG_FMT_ERROR(logger, fmt, ...) SHUAI_LOG_FMT_LEVEL(logger, shuai::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SHUAI_LOG_FMT_FATAL(logger, fmt, ...) SHUAI_LOG_FMT_LEVEL(logger, shuai::LogLevel::FATAL, fmt, __VA_ARGS__)

#define SHUAI_LOG_ROOT() shuai::LoggerMgr::GetInstance()->getRoot()

namespace shuai
{
class Logger;
class LogLevel
{
public:
    enum Level
    {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    static const char* ToString(LogLevel::Level level);
};

// 日志事件
class LogEvent
{ 
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse
            , uint32_t thread_id, uint32_t fiber_id, uint64_t time);

    const char* getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getElapse() const { return m_elapse; }
    uint32_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint64_t getTime() const { return m_time; }
    std::string getContent() const { return m_ss.str(); }
    std::shared_ptr<Logger> getLogger() const { return m_logger; }
    LogLevel::Level getLevel() { return m_level; }

    std::stringstream& getSS() { return m_ss; }
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);
private:
    const char* m_file = nullptr;   // 文件名
    int32_t m_line = 0;             // 行号
    uint32_t m_elapse = 0;          // 程序启动到现在的毫秒数
    uint32_t m_threadId = 0;        // 线程号
    uint32_t m_fiberId = 0;         // 协程号
    uint64_t m_time;                // 时间戳
    std::stringstream m_ss;         // 消息内容 将消息输入到这个流中

    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
};

class LogEventWrap
{
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();

    std::stringstream& getSS();
    LogEvent::ptr getEvent() { return m_event; }
private:
    LogEvent::ptr m_event;
};

// 日志格式器
class LogFormatter
{   
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);    // 测试时是将这个传了过来"%d  [%p]  <%f:%l>   %m %n"
    // %t    %thread_id %m%n
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
public:
    class FormatItem
    {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() {}
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

    void init();
private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
};

// 日志输出地
class LogAppender
{
public:
    typedef std::shared_ptr<LogAppender> ptr;

    virtual ~LogAppender() {} 
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    void setFormatter(LogFormatter::ptr val) { m_formatter = val; }
    LogFormatter::ptr getFormatter() const { return m_formatter; }
    void setLevel(LogLevel::Level level) { m_level = level; }
    LogLevel::Level getLevel() const { return m_level; }
protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    // std::shared_ptr<int> sp; 和 std::unique_ptr<int> up; 创建了未初始化的智能指针，默认情况下它们的值为 nullptr
    LogFormatter::ptr m_formatter;
};

// 日志器
class Logger: public std::enable_shared_from_this<Logger>
{
public:
    typedef std::shared_ptr<Logger> ptr;

    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, const LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }
    const std::string& getName() const { return m_name; }
private:
    std::string m_name;                             // 日志名称
    LogLevel::Level m_level;                        // 日志级别
    std::list<LogAppender::ptr> m_appenders;        // Appender是个列表
    LogFormatter::ptr m_formatter;
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender
{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;

};

// 输出到文件的Appender
class FileLogAppender : public LogAppender
{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;

    // 重新打开文件，重新打开成功返回true
    bool reopen();
private:
    std::string m_filename;  // 文件名
    std::ofstream m_filestream;
};

class LoggerManager
{
public:
    LoggerManager();
    void init();
    Logger::ptr getRoot() const { return m_root; }
    Logger::ptr getLogger(const std::string& name);

private:
    std::map<std::string, Logger::ptr> m_logger;
    Logger::ptr m_root;
};

typedef shuai::Singleton<LoggerManager> LoggerMgr;

}