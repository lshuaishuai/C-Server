#include <iostream>
#include <thread>

#include "../server/util.h"
#include "../server/log.h"

int main(int argc, char** argv)
{
    shuai::Logger::ptr logger(new shuai::Logger);   // 默认构造
    logger->addAppender(shuai::LogAppender::ptr(new shuai::StdoutLogAppender));  // 父类指针指向子类对象发生多态 这里的默认日志级别为DEBUG

    shuai::FileLogAppender::ptr file_appender(new shuai::FileLogAppender("./log.txt"));

    shuai::LogFormatter::ptr fmt(new shuai::LogFormatter("%d%T%p%T%m%n")); // 设置formatter，不使用logger里面的
    file_appender->setFormatter(fmt);
    file_appender->setLevel(shuai::LogLevel::ERROR);  // 只有大于等于ERROR的才会被设置经file_appender中

    logger->addAppender(file_appender);  // 上面已经设置好formater，这个函数中就不会重新设置了
    // shuai::LogEvent::ptr event(new shuai::LogEvent(__FILE__, __LINE__, 0, shuai::GetThreadId(), shuai::GetFiberId(), time(0)));
    // event->getSS() << "hello shuai log";  // 还有个问题 为什么会在日志后面输出呢？  --- 是将它输入到了strinstream流里面  当遇到%m的时候就会将这条消息从LogEvent::m_ss流中读出来

    // logger->log(shuai::LogLevel::DEBUG, event);  

    // 定义了几个日志输出地就会根据日志级别将日志输出到几个地方
    // 现在下面的日志不需要像上面一样显式的调用 logger::log了 会在宏函数中自己调用
    SHUAI_LOG_INFO(logger) << "test macro";    // 这句话完了之后LogEventWrap匿名对象就会析构，从而执行log，将流中的数据输入到命令行或者文件中
    SHUAI_LOG_ERROR(logger) << "test macro error";

    SHUAI_LOG_FMT_ERROR(logger, "test marco fmt error %s", "aa");

    auto l = shuai::LoggerMgr::GetInstance()->getLogger("xx");
    SHUAI_LOG_INFO(l) << "xxx";
    return 0;
}