 # C-server
 本仓库为重新写了sylar的C++高性能服务器框架，并加了必要的注释，感兴趣的小伙伴可以一起交流
 
 # 日志系统
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
 # 配置系统
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

 # 日志系统整合配置系统
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

