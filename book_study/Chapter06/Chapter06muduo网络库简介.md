#Chapter06
##6.3目录结构
>muduo        muduo库的主体
>>base        与网络无关的基础代码，位于::muduo namespace，包括线程库   
>>net         网络库，位于::muduo::net namespace
>>>poller     poll(2) 和 epoll(4)两种IO multiplexing后端   
>>>http       一个简单的可嵌入的Web服务器   
>>>inspect    基于以上Web服务器的“窥视器”，用于报告进程的状态   
>>>protorpc   简单实现Google Protobuf RPC，不推荐

基础库
>muduo
>>base
>>>AsyncLogging.{h,cc} 异步日志backend     
>>>Atomic.h 原子操作与原子整数   
>>>BlockingQueue.h 无界阻塞队列(生产者消费者队列)   
>>>Condition.h 条件变量，与Mutex.h一同使用     
>>>copyable.h 一个空基类，用于标识（tag）值类型   
>>>CountDownLatch.{h,cc} “倒计时门闩”同步   
>>>Date.{h,cc} Julian日期库（即公历）   
>>>Exception.{h,cc} 带static trace的异常基类   
#[懒]  
![基础库](http://www.baidu.com/img/bdlogo.gif "基础库")
![网络核心库](http://www.baidu.com/img/bdlogo.gif "网络核心库")
![网络附属库](http://www.baidu.com/img/bdlogo.gif "网络附属库")
### 6.3.1代码结构
![代码结构01](http://www.baidu.com/img/bdlogo.gif "代码结构01")
![代码结构02](http://www.baidu.com/img/bdlogo.gif "代码结构02")