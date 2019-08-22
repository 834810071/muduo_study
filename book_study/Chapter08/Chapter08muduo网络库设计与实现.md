#Chapter08

>> 1. &8.1至&8.3介绍Reactor模式的现代C++实现，包括EventLoop、Poller、Channel、TimerQueue、EventLoopThread等class。   
>> 2. &8.4至&8.9介绍基于Reactor的单线程、非阻塞、并发TCP server网络编程，主要介绍Acceptor、Socket、TcpServer、TcpConnection、Buffer等class。  
>> 3. &8.10至&8.13介绍one loop per thread的实现(用EventLoopThreadPool实现多线程TcpServer)，Connector和TcpClient class，还有用epoll(4)替换poll(2)作为Poller的IO multiplexing机制等。

##8.0 什么都不做的EventLoop
>> 首先定义EventLoop class的基本接口：构造函数、析构函数、loop()成员函数。  
>> one loop per thread顾名思义每个线程只能有一个EventLoop对象，因此EventLoop的构造函数会检查当前线程是否创建了其他EventLoop对象。EventLoop的构造函数会记住本对象所属的线程（threadId_）。创建EventLoop对象的线程是**IO线程**，其主要功能是运行事件循环EventLoop::loop()。EventLoop对象的生命周期和其所属的线程一样长，它不必是heap对象。
[select、poll、epoll之间的区别总结](https://www.cnblogs.com/Anker/p/3265058.html)   
![poll事件类型](https://github.com/834810071/muduo_study/blob/master/book_study/poll%E4%BA%8B%E4%BB%B6%E7%B1%BB%E5%9E%8B "poll事件类型")

## 8.1 Rector的关键结构
>>Reactor最核心的事件分发机制，即将IO multiplexing拿到的IO事件分发给各个文件描述符(fd)的事件处理函数。  
### 8.1.1 Channel class
>> 每个Channel对象自始至终只负责一个文件描述符(fd)的IO事件分发，但它并不拥有这个fd，也不会在析构的时候关闭这个fd。Channel会把不同的IO事件分发为不同的回调，例如ReadCallback、WriteCallback等，而且回调用boost::function表示，用户无须继承Channel，Channel不是基类。muduo用户一般不直接使用Channel，而会使用更上层的封装，如TcpConnection。Channel的生命期由其owner class负责管理，它一般是其他class的直接或间接成员。   
  
> Channel::handleEvent()是Channel的核心，它由EventLoop::loop()调用，功能是根据revent_的值分别调用不同的用户回调。

### 8.1.2 Poller class
>> Poller class 是IO multiplexing的封装。在muduo中是个抽象基类，因为muduo支持poll(2)和epoll(4)两种IO multiplexing机制。Poller是EventLoop的间接成员，只供其owner EventLoop在IO线程调用，因此无须枷锁。其生命周期与EventLoop相同。Poller并不拥有Channel，Channel在析构前必须自己unregister(EventLoop::removeChannel())，避免空悬指针。   
>> Poller供EventLoop调用的函数目前有两个，poll()和updateChannel()。   
>> Poller数据成员：其中ChannelMap是从fd到Channel*的映射。Poller::poll()不会在每次调用poll(2)之前临时构造pollfd数组，而是把它缓存起来(pollfds_)。       

> Poller::poll()是Poller的核心，它调用poll(2)获得当前获得的IO事件，然后填充调用方传入的activeChannels，并返回poll(2)return的时刻。  
>> Poller的职责：只负责IO multiplexing，不负责事件的分发(dispatching)。
