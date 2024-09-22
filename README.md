 # C-server
 
 ## 日志系统
 在这个部分我们仿照log4j的模式实现了日志系统，一共有六个级别(UNKNOW、DEBUG、INFO、WARN、ERROR、FATAL)。主要有三大模块Logger(日志器)、LogAppender(日志输出地)、 
 LogFormat(日志输出格式)。  
 Logger类是对外使用的接口类，只有输入的日志级别大于等于Logger的日志级别才会被输出。可以定义不同的Logger记录不同类型的日志。  
 LoggerAppender类是日志输出地类，我们实现了两个子类我们写了，将日志输出到控制台(StdoutLogAppender)和输出到文件(FileLogAppender)中。  
 LogFormat类是日志的格式类，可以使用默认的格式或者是自定义格式。其中设置了默认日志标识符，不同的字母对应需要输出不同的内容。该类中还包括FormatItem类，通过多态的方式来实 
 现了不同格式调用不同的子类方法。  
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
 LogEvent类将要写的日志填充到该类中，然后使用Logger对其进行日志格式的转换。  
 使用LoggerManager来管理所有的Logger。并且使用宏来进行日志的打印，不需要自己显式的创建类实例。  
 ## 配置系统
