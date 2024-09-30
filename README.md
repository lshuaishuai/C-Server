 # C-server
 本仓库为重新写了sylar的C++高性能服务器框架，并加了必要的注释，感兴趣的小伙伴可以一起交流
 
 ## 日志系统
 在这个部分我们仿照log4j的模式实现了日志系统，一共有六个级别(UNKNOW、DEBUG、INFO、WARN、ERROR、FATAL)。主要有三大模块Logger(日志器)、LogAppender(日志输出地)、 
 LogFormat(日志输出格式)。  
 Logger类是对外使用的接口类，只有输入的日志级别大于等于Logger的日志级别才会被输出。可以定义不同的Logger记录不同类型的日志。  
 LoggerAppender类是日志输出地类，我们实现了两个子类我们写了，将日志输出到控制台(StdoutLogAppender)和输出到文件(FileLogAppender)中。  
 LogFormat类是日志的格式类，可以使用默认的格式或者是自定义格式。其中设置了默认日志标识符，不同的字母对应需要输出不同的内容。该类中还包括FormatItem类，通过多态的方式来实 
 现了不同格式调用不同的子类方法。  
 ```cpp
 %d{%Y-%m-%d %H:%M:%S} : %d 标识输出的是时间 {%Y-%m-%d %H:%M:%S}为时间格式，可选 DateTimeFormatItem  
 %T : Tab[\t]            TabFormatItem  
 %t : 线程id             ThreadIdFormatItem  
 %N : 线程名称           ThreadNameFormatItem  
 %F : 协程id             FiberIdFormatItem  
 %p : 日志级别           LevelFormatItem         
 %c : 日志名称           NameFormatItem  
 %f : 文件名             FilenameFormatItem  
 %l : 行号               LineFormatItem  
 %m : 日志内容           MessageFormatItem  
 %n : 换行符[\r\n]       NewLineFormatItem
```
 LogEvent类将要写的日志填充到该类中，然后使用Logger对其进行日志格式的转换。  
 使用LoggerManager来管理所有的Logger。并且使用宏来进行日志的打印，不需要自己显式的创建类实例。  
 ## 配置系统
 yaml库的安装、将yaml和配置系统整合  
 配置系统的原则，约定优于配置：  
 ```cpp  
 template<T, FromStr, ToStr>
 class ConfigVar;

 template<F, T>
 LexicalCast;
   
 // 复杂类型的支持   
 // 容器偏特化(vector, list, map, set, unordered_map, unirdered_set map只支持key为string类型的数据，) 目的是为了实现STL容器到string的相互转换
 // Config::Lookup(key), key相同，类型不同的不会报错，但是得解决
 ```
 自定义类型，需要实现偏特化， 实现后就可以支持Config解析自定义类型，自定义类型可以和常规stl容器一起用

 配置的事件机制：当一个配置项发生修改的时候，可以反向通知对应的代码(回调)。

 ## 日志系统整合配置系统
 ```yaml
 logs:
     - name: root
       level: (debug, info, warn, error, fatal)
       formatter：'%d%T%p%T%t%m%n'
       appender: 
           - type: (StdoutLogAppender, FileLogAppender)
             level: (...)
             file: /logs/xxx.log
```      
```cpp
    shuai::Logger g_logger = 
    shuai::LoggerMgr::GetInstance()->getLogger(name);
    SHUAI_LOG_INFO(g_logger) << "xxx log"; 
```

```cpp
    static Logger::ptr g_log = SHUAI_LOG_NAME("system");
    // m_root, m_system->root
    当logger的appenders为空，使用root写logger
```

```cpp
// 定义LogDefine LogAppenderDefine， 偏特化 LexicalCast，
// 实现日志配置解析
```
遗留问题：
1. appender定义的formatter读取yaml的时候，没有被初始化
解决了遗留问题，是没有setFormatter

下面为简单的日志系统和配置系统的UML类图
![sylar服务器框架类图](https://github.com/user-attachments/assets/5cc0d558-8701-41ae-b68f-c371ce147374)

## 线程库

Thread, Mutex
// 为了区分读写锁减少性能损失，我们使用系统调用来实现Mutex 

互斥量 mutex
信号量 semaphore

// 该代码实现了一个高效的读写锁机制，允许多个线程同时进行读取，而在写入时则需要独占锁

解决日志代码线程安全的问题 -- 将线程互斥量信号和日志模块 整合
Logger、Appender、Loggermanger都加上锁

加锁以后，写日志的速度比不加锁要慢很多，进行优化。 ---  经过测试 Mutex比spinlock和CASLock都快
还做了每个一段时间重新打开文件的操作 -- 周期性reopen

将Config和互斥量结合做到线程安全

## 协程库封装
定义协程接口
ucontext
26： 做了一些协程之前的准备工作，assert宏和backtrace函数
```
27:  Thread--->main_fiber <-------> sub_fiber         // 每个线程有一个主协程，这个main协程可以创建、调度协程   子协程执行完或者让出CPU后回到main协程 
                   ^                                  // 也就是说在子协程之间调度时，只能是 子协程1->主协程->子协程2  不能直接 子协程1->子协程2
                   |   创建协程要回到main_fiber中
                   v
              sub_fiber
```              
```cpp
static thread_local Fiber* t_fiber = nullptr;                        // 保存当前正在运行的协程指针，必须时刻指向当前正在运行的协程对象。协程模块初始化时，t_fiber指向线程主协程对象
static thread_local Fiber::ptr t_threadFiber = nullptr;  // main协程 // 保存线程主协程指针，智能指针形式。协程模块初始化时，t_thread_fiber指向线程主协程对象。当子协程resume时，通过swapcontext将主协程的上下文保存到t_thread_fiber的ucontext_t成员中，同时激活子协程的ucontext_t上下文。当子协程yield时，从t_thread_fiber中取得主协程的上下文并恢复运行

```
