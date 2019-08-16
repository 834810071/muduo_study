# Chapter01

## 1.1当析构函数遇到多线程

### 1.1.2 MutexLock与MutexLockGuard

>> 代码见&2.4
>> MutexLock封装临界区（critical section），这是一个简单的资源类，用RAII(RAII全称是“Resource Acquisition is Initialization”，直译过来是“资源获取即初始化”,也就是说在构造函数中申请分配资源，在析构函数中释放资源)手法封装互斥器的创建与销毁。在Linux下是pthread_mutex_t，默认是不可重入的（&2.1.1）。MutexLock一般是别的class的数据成员。   

[RAII参考链接](https://www.cnblogs.com/jiangbin/p/6986511.html)

>> MutexLockGuard封装临界区的进入和退出，即加锁和解锁。MutexLockGuard一般是个栈上的对象，它的作用域刚好等于临界区域。

>> 这两个class都不允许拷贝构造和赋值，使用原则见&2.1。

### 1.1.3 一个线程安全的Counter示例
[关键字mutable](https://blog.csdn.net/aaa123524457/article/details/80967330)
[关键字const](https://www.cnblogs.com/kevinWu7/p/10163449.html)

## 1.4 线程安全的Oberver有多难
>> 在面向对象程序设计中，对象的关系主要有三种：composition(组合/复合)，aggregation(关联/联系)，association(聚合)。

[智能指针boost::scoped_str](https://www.cnblogs.com/helloamigo/p/3572533.html) [关键字explicit](https://blog.csdn.net/guoyunfei123/article/details/89003369)



