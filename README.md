# sylar协程网络库的学习
## 日志模块
日志模块主要有四个类：Logger,LogEvent,LogFormatter,LogAppender,另外有两个辅助作用的类，分别是LogEventWrap和LoggerManager,各个类的职责如下：  
- LogEvent:保存当前Log的现场信息，譬如文件名,行号,函数名...
- LogEventWrap:RAII管理LogEvent,析构时将LogEnvent落地当log中
- Logger:日志器,用户真正使用的类，负责将LogEvent落地到输出地
- LogFormatter:日志格式,将LogEvent格式化为指定格式
- LogAppender:日志落地类,决定日志的输出地,可以是文件，可以使stdout  

日志构造的整体流程:  
```
 +---------+
 |LogEvent |     ----->构造现场信息
 +---------+
      |
      v
+-------------+
|LogEventWrap |  ----->析构时自动调用Log方法
+-------------+
      |
      V
   +-------+
   |Logger |     ----->将LogEvent交给具体日志器
   +-------+
      |
      V
+-------------+
|LogAppender |   ----->一个Logger可能有多个输出地
+-------------+
      |
      V
+-------------+
|LogFormatter |  ----->将LogEvent格式化
+-------------+
      |
      V
输出到具体输出地
```
为简化使用，借助宏实现了流式输出和格式化输出两种方法：  
```
// 流式输出
#define XZMJX_LOG_LEVEL(logger,level) \
if(logger->getLevel()<=level)         \
xzmjx::LogEventWrap(xzmjx::LogEvent::ptr(new xzmjx::LogEvent(logger,level,__FILE__,__LINE__, \
                                        0,xzmjx::GetThreadID(),xzmjx::GetFiberID(),          \
                                        static_cast<uint64_t>(time(NULL)),xzmjx::Thread::GetName()))).getSS()

// 格式化输出
#define XZMJX_LOG_FMT_LEVEL(logger,level,fmt,...) \
if(logger->getLevel()<=level)         \
xzmjx::LogEventWrap(xzmjx::LogEvent::ptr(new xzmjx::LogEvent(logger,level,__FILE__,__LINE__, \
                                        0,xzmjx::GetThreadID(),xzmjx::GetFiberID(),          \
                                        static_cast<uint64_t>(time(NULL)),xzmjx::Thread::GetName()))).getEvent()->format(fmt,__VA_ARGS__)

```

## 配置模块
约定优于配置  
Config-->Yaml作为底层支持  
定义即配置,每个配置项有一个默认的约定，采用静态变量的方式预先定义好，读取配置文件时，没有对应的值则使用默认值，有配置的值则使用配置的值。  
模块主要实现的方法：  
string与复杂类型的相互转换规则  
写入YAML文件和读取YAML文件  

### 实现多常用数据类型和自定义类型的支持
主要通过实现FromString和ToString两种方法,借助模板类的偏特化,为具体类型定制解析方法,且可以叠加使用：  
```
vector<list<int>> ---> string
string ---> vector<list<int>>
```
只要分别实现了vector和list的转化规则，便自然可以解析上面的复杂类型。  
具体实现大致如下(以vector为例)
```
//从string中解析YAML
YAML::Node n = YAML::Load(s);
// 调用T的fromString方法,这里实现为一个仿函数的类,并为T实现一个偏特化
// 便实现了叠加使用
typename std::vector<T> ans;
std::stringstream ss;
for(size_t i = 0;i<node.size();i++){
    ss.str("");
    ss<<node[i];
    ans.push_back(LexicalCast<std::string,T>()(ss.str()));
}
```
## 线程模块
简单将pthread封装了一下：
```
Thread(std::function<void()> cb,std::string name);
```
接受一个std::function<void()>的线程执行体，虽然是void(),但由于标准库提供的有bind函数，故可以看成接受任何参数的执行函数。  
另外一个有趣的点在于：  
```
Thread::Thread(std::function<void()> cb, std::string name):m_cb(move(cb)),
                                                            m_name(move(name)),
                                                            m_tid(0),
                                                            m_thread(0){
    if(m_name.empty()){
        m_name = "UNKOWN";
    }
    int ret = pthread_create(&m_thread,NULL,Thread::run, this);
    if(ret != 0){
        XZMJX_LOG_ERROR(g_logger)<<"pthread_create failed,error = "<<ret<<" thread name = "<<m_name;
        throw std::logic_error("pthread_create error");
    }
    m_sem.wait();
}
```
构造函数最后`m_sem.wait()`,这个信号量初始为0，在run函数中执行p操作，此处的v操作才会返回，因此可以得出，构造函数返回时，也保证了线程执行函数一定跑起来了。

## 协程模块
### 协程相关知识学习

### 协程开源库libco使用

### u_context相关接口

### 协程模块封装


## 协程调度模块
在协程模块封装的是一个非对称的协程，每个线程有两个关于协程的线程局部变量，一个是当前线程上正在跑的协程，一个是当前线程的主协程。故在任一个协程在进行切换时，都需要与主协程进行上下文切换，再由主协程与其余子协程进行上下文切换。

假设我们的调度模块设计为，一个线程创建调度器，并且往调度器里分配任务，调度器内有多个线程，每个线程都执行调度工作，具体来说便是向任务队列中取任务，再切换上下文去执行任务，那么当前的设计便还能正常工作，目前的调度还是只涉及两个协程，对于调度线程而言，其上的主协程便是调度协程，当调度器工作时，便在调度协程与工作协程之间切换，不会出现问题，而构造调度器的线程虽然主协程不是调度协程，但它不参与子协程的调度工作，所以也不会出现问题。

以上的假设在目前`sylar`的设计中不成立，因为框架支持`use_caller`选项，启用此选项便意味着，创建调度器的线程也参与调度，那此时这个线程上需要另外一个协程来跑调度器的工作，这个线程上便出现了三类协程：**主协程---调度协程---工作协程**，其中**主协程**是该线程上第一个协程，跑的是这个线程本身，**调度协程**跑的是调度器，**工作协程**跑的是工作队列中取的任务，这样一来调度时的上下文切换便出现了困难，如上面所说，上下文切换时只支持从工作协程切到主协程，这样以来便不能进行调度的任务了。

`sylar`解决这个问题的方式是，增加一个调度协程的线程局部变量，对于一般的调度调度协程，其主协程便是调度协程，故`t_main_fiber == t_scheduler_fiber`,这类线程的协程切换在工作协程与`t_scheduler_fiber`之间进行即可，而对于启用了`use_caller`的`caller`线程，其上的主协程不是调度协程，故有`t_main_fiber != t_scheduler_fiber`,此时工作协程与`t_scheduler_fiber`切换，`t_scheduler_fiber`与`t_main_fiber`切换。

自己在编写协程调度模块时，对sylar的协程调度做了简化，不支持使用caller线程也作为调度协程，从而简化了逻辑已经编码难度。。。


## IO协程调度模块
前面实现的协程调度在对于cpu的利用上过于浪费，譬如在无任务时，调度线程会运行wait协程，该协程的设计只是进行忙等，不断地切换进出去判断是否有任务，而里面的用来通知其他调度线程尚有任务待完成的notify并没有任何作用，即没有一个合理的等待通知机制来充分的利用cpu，这就造成了资源的浪费。而IO协程调度模块借助epoll的机制，将wait协程等在epoll_wait上。wait协程在三种情况下让出执行权，一是有新的任务放入任务队列，二是超时事件触发，三是观察的文件描述符有事件发生，都是通过epoll_wait返回来实现的。

## hook模块
hook实际上就是对系统调用API进行一次封装，将其封装成一个与原始的系统调用API同名的接口，应用在调用这个接口时，会先执行封装中的操作，再执行原始的系统调用API。
hook技术可以使应用程序在执行系统调用之前进行一些隐藏的操作，比如可以对系统提供malloc()和free()进行hook，在真正进行内存分配和释放之前，统计内存的引用计数，以排查内存泄露问题。
例如将读写操作hook住，虽然socket都被设置为非阻塞，但通过hook将可能阻塞的读写协程yield出去，对外展现一种同步阻塞的效果，即程序可以执行read->process->write这样的同步操作。


##网络部分
### Address封装

### Socket封装

### ByteArray
#### ZigZag算法
补码表示：小整数前导零较多，负数前导一较多，可压缩这部分数据,通过如下算法将符号位后置，对于正数而言前面的数字不变，负数而言前面的数字取反
```go
func int32ToZigZag(n int32) int32 {
return (n << 1) ^ (n >> 31)
}
```
ZigZag还原：相当于上述的逆操作
```go
func toInt32(zz int32) int32 {
    return int32(uint32(zz)>>1) ^ -(zz & 1)
}
```
```
-2-->10000010-->11111110(补码)-->11111101-->00000011(zigzag)
```
可以看出-1被编码为1,1被编码为2,-2被编码为3,2被编码为4，正负交错为锯齿状，故称为zigzag

```
00000000000000000000000000000011
               |
               |
               v
0000000 | 0000000 | 0000000 | 0000000 | 0011 --> 00000011(zigzag最终)
```
使用zigzag编码之后,可使用varint进行压缩,具体为：分为7位一组，加上最高位表示是否为最后一组。组成一个字节数组

### 




