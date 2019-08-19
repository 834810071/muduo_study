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

[懒]    
![基础库](https://github.com/834810071/muduo_study/blob/master/book_study/%E5%9F%BA%E7%A1%80%E5%BA%93.png "基础库") 

网络核心库   

![网络核心库](https://github.com/834810071/muduo_study/blob/master/book_study/%E7%BD%91%E7%BB%9C%E6%A0%B8%E5%BF%83%E5%BA%93.png "网络核心库")   
网络附属库     
![网络附属库](https://github.com/834810071/muduo_study/blob/master/book_study/%E7%BD%91%E7%BB%9C%E9%99%84%E5%B1%9E%E5%BA%93.png "网络附属库")  
  
### 6.3.1代码结构
代码结构

![代码结构01](https://github.com/834810071/muduo_study/blob/master/book_study/%E4%BB%A3%E7%A0%81%E7%BB%93%E6%9E%8401.png "代码结构01")     
![代码结构02](https://github.com/834810071/muduo_study/blob/master/book_study/%E4%BB%A3%E7%A0%81%E7%BB%93%E6%9E%8402.png "代码结构02")   