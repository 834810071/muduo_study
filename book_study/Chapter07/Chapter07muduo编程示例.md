#Chapter07
##7.3 Boost.Asio的聊天服务器
###7.3.1 TCP分包
>>对于长连接的TCP服务，分包有四种办法：
>>1. 消息长度固定，比如muduo采用了固定的16字节消息。
>>2. 使用特殊字符或字符串作为消息的边界，例如HTTP协议的headers以"\r\n"作为字段的分隔符。
>>3. 在每条消息的头部加一个长度字段。
>>4. 利用消息本身的格式来分包，例如json。解析这种消息格式通常会用到状态机(state machine)。
## 7.4 muduo Buffer类的设计与使用
###7.4.1 muduo的IO模型
>>Linux上的五种IO模型：阻塞（blocking）、非阻塞（non-blocking）、IO复用（IO multiplexing）、信号驱动（signal-driven）、异步（asynchronous）。这些都是单线程下的IO模型。   
  
>> one loop per thread is usually a good model。

### 7.4.2
>> muduo EventLoop采用的是epoll(4) level trigger，而不是edge trigger。

### 7.4.3 Buffer的功能需求
>>muduo Buffer的设计要点   
>> * 对外表现为一块连续的内存(char* p, int len)，以方便客户代码的编写。
>> * 其size()可以自动增长，以适应不同大小的消息。它不是一个fixed size array。
>> * 内部以std::vector<char> 来保存数据，并提供相应的访问函数。

>> TcpConnection会有两个Buffer成员，input buffer 和 output buffer。
>> * input buffer，TcpConnection会从socket读取数据，然后写入input buffer；客户代码从input buffer读取数据。
>> * outpu buffer，客户代码会把数据写入output buffer；TcpConnection从output buffer读取数据并写入socket。

Buffer类图   

![Buffer类图]( "Buffer类图")

Muduo Buffer 类的数据结构    

![Buffer 类的数据结构]( "Buffer类的数据结构")

>>两个index把vector的内容分为三块：prependable、readable、writable。灰色部分是Buffer的有效载荷(payload)。   
>> prepenable = readIndex； readable = writeIndex - readIndex； writable = size() - writeIndex。

Buffer初始化数据结构图   

![Buffer初始化数据结构图]( "Buffer初始化数据结构图")

### 7.4.5Buffer的操作
**基本的read-write cycle**
>>向Buffer写入200字节后的布局   
![向Buffer写入200字节后的布局]( "向Buffer写入200字节后的布局")

   
     
