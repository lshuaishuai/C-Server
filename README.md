44 # C-server
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

27:  Fiber::GetThis(); // 若当前线程没有协程，会创建一个主协程
     Thread--->main_fiber <-------> sub_fiber         // 每个线程有一个主协程，这个main协程可以创建、调度协程   子协程执行完或者让出CPU后回到main协程 
                   ^                                  // 也就是说在子协程之间调度时，只能是 子协程1->主协程->子协程2  不能直接 子协程1->子协程2
                   |   创建协程要回到main_fiber中
                   v
              sub_fiber
```              
```cpp
static thread_local Fiber* t_fiber = nullptr;                        // 保存当前正在运行的协程指针，必须时刻指向当前正在运行的协程对象。协程模块初始化时，t_fiber指向线程主协程对象
static thread_local Fiber::ptr t_threadFiber = nullptr;  // main协程 // 保存线程主协程指针，智能指针形式。协程模块初始化时，t_thread_fiber指向线程主协程对象。当子协程resume时，通过swapcontext将主协程的上下文保存到t_thread_fiber的ucontext_t成员中，同时激活子协程的ucontext_t上下文。当子协程yield时，从t_thread_fiber中取得主协程的上下文并恢复运行
a
```

## 协程调度模块
```
         1 -> N     1 -> M
协程调度模块支持多线程，支持使用caller线程进行调度，支持添加函数或协程作为调度对象，并且支持将函数或协程绑定到一个具体的线程上执行。在使用caller线程的情况下，线程数自动减一，并且调度器内部会初始化一个属于caller线程的调度协程并保存起来（比如，在main函数中创建的调度器，如果use_caller为true，那调度器会初始化一个属于main函数线程的调度协程）。
当不适用caller线程进行调度时，在一个线程中每次进行协程的切换都要经过fiberid=0的协程，即主协程；而使用caller线程进行调度时，这个线程并不是使用fiberid为0的协程，调度协程并不是caller线程的主协程，而是相当于caller线程的子协程，而调度协程和执行任务的协程都为字写成不能直接切换否则会跑飞，那么这个就是关键点了如何进行切换呢？我们可以将caller线程的协程切换单独拎出来，先将其从主协程切换到调度携程，再在调度协程中进行切换，最后退出的时候从调度协程切换到主协程。
停止调度器：如果use_caller为true，表示caller线程也要参于调度，这时，调度器初始化时记录的属于caller线程的调度协程就要起作用了，在调度器停止前，应该让这个caller线程的调度协程也运行一次，让caller线程完成调度工作后再退出。如果调度器只使用了caller线程进行调度，那么所有的调度任务要在调度器停止时才会被调度。
scheduler --> thread --> fiber
1.线程池，分配一组线程
2.协程调度器，将协程指定到对应的线程上去执行
N : M
m_threads
<function<void()>, fiber, threadid> m_fibers

scheduler(func/fiber)

start()
stop()
run()

1.设置当前线程的scheduler
2.设置当前线程的run、fiber
3.协程调度循环while(true)
    a. 协程消息队列里面是否有任务
    b. 无任务，执行idle
```

## IO协程调度器
使前面实现的调度可以实现异步IO
```
IOManager(epoll) ---> Scheduler
      |
      V
    idle(epoll_wait)
一般的线程池： message_queue 使用信号量来通知线程有消息到来 ，若无任务到来，该信号量会将整个线程阻塞(但是我们并不想这样)
异步IO：等待数据返回时，epoll_wait等待
```

## 定时器
```
定时器可以实现给服务器注册定时事件，这是服务器上经常要处理的一类事件，比如3秒后关闭一个连接，或是定期检测一个客户端的连接状态
有两种高效管理定时器的容器：时间轮和时间堆，我们使用时间堆的方式管理定时器
实现思路是直接将超时时间当作tick周期，具体操作是每次都取出所有定时器中超时时间最小的超时值作为一个tick，这样，一旦tick触发，超时时间最小的定时器必然到期。处理完已超时的定时器后，再从剩余的定时器中找出超时时间最小的一个，并将这个最小时间作为下一个tick，如此反复，就可以实现较为精确的定时。
Timer -> addTimer() --> cancel()
获取当前的定时器触发离现在的时间差
返回当前需要触发的定时器
```
```
            [Thread]            [Timer]
               ^N                  ^
               |1                  |
            [fiber]          [TimerManager]
               ^M                  ^
               |1                  |
          [Scheduler] <-- [IOManager(epoll)]
```

## HOOK
hook实际上就是对系统调用API进行一次封装，将其封装成一个与原始的系统调用API同名的接口，应用在调用这个接口时，会先执行封装中的操作，再执行原始的系统调用API。
hook技术可以使应用程序在执行系统调用之前进行一些隐藏的操作，比如可以对系统提供malloc()和free()进行hook，在真正进行内存分配和释放之前，统计内存的引用计数，以排查内存泄露问题。
hook的目的是在不重新编写代码的情况下，把老代码中的socket IO相关的API都转成异步，以提高性能。hook和IO协程调度是密切相关的，如果不使用IO协程调度器，那hook没有任何意义.
我们之前实现的代码，协程只能按顺序调度，一旦有一个协程阻塞住了，那整个调度线程也就阻塞住了，其他的协程都无法在当前线程上执行。所以我们要对接口进行hook。
hook的函数签名与原函数一样，所以对调用方也很方便，只需要以同步的方式编写代码，实现的效果却是异步执行的，效率很高。
sleep,
usleep

与socket相关的(socket accept connect)
io相关(read\write\recv\send......)
fd相关(fcntl ioctl .......)

## socket函数库
                   [UnixAddress]
                         |
                     ---------                   |--[IPv4Address]
                     |Address| --- [IPAddress] --|
                     ---------                   |--[IPv6Address]
                         |
                         |             
                     ----------
                     | Socket |
                     ----------

## 序列化bytearray

write(int/float/int64_t/...)  
read(int/float/int64_t/...)

## http协议开发
HTTP/1.1 - API  
  
HttpRequest;  
HttpResponse;  

GET /uri HTTP/1.1  
Host: www.sylar.top  

HTTP/1.0 200 OK  
Pragma: no-cache  
Content-Type: text/html  
Content-Length: 14988  
Connection: close  

uri: http://www.sylar.top/page/xxx?id=10&v=20#fr  
     http: 协议  
     www.sylar.top: host  
     80: 端口  
     /page/xxx: path  
     id=10&v=20: param  
     fr: fragment  

## TcpServer封装
基于TcpServer实现了一个EchoServer

## Stream封装 针对文件/Socket的封装
read/write/readFix/writeFixSize

HttpSession/HttpConnection
Server.accept, socket -> session
Client.connect socket -> Connecion

HttpServer : TcpServer

         Serlvet <----- FunctionServlet
            |
            V
     ServletDispatch

## 守护进程 deamon
```cpp
fork
  | ---- 子进程 --> 执行server
  | ---- 父进程 --> wait(pid)
```

## 输入参数解析
int argc, char** argv
./shuai -d -c 

## 环境变量
getenv
setenv
/proc/pid/cmdline|cwd|exe

利用/proc/oid/cmdline 和全局变量构造函数，实现在进入main函数前解析参数
1. 读写环境变量
2. 获取程序的绝对路径，基于绝对路径设置cwd
3. 可以通过cmdline，在进入main函数之前解析好参数

## 配置加载
配置的文件夹路径，log.yaml http.yml tcp.tml thread.yml
```cpp
```

## Server主体框架
1. 防止重复启动多次
2. 初始化日志文件路径
3. 工作目录的路径
