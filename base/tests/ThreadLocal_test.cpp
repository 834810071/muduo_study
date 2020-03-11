//
// Created by jxq on 19-7-2.
//
#include "../ThreadLocal.h"
#include "../CurrentThread.h"
#include "../Thread.h"

#include <stdio.h>

class Test : muduo::noncopyable {
public:
    Test() {
        printf("tid = %d, constructing %p\n", muduo::CurrentThread::tid(), this);
    }

    ~Test() {
        printf("tid = %d, destructing %p %s\n", muduo::CurrentThread::tid(), this, name_.c_str());
    }

    // const 后
    // (1)const修饰的成员函数不能修改任何的成员变量(mutable修饰的变量除外)
    // (2)const成员函数不能调用非onst成员函数，因为非const成员函数可以会修改成员变量

    // const 前
    // 如果返回const data,non-const pointer，返回值也必须赋给const data,non-const pointer。因为指针指向的数据是常量不能修改。
    const muduo::string &name() const {
        return name_;
    }

    void setName(const muduo::string &n)
    {
        name_ = n;
    }

private:
    muduo::string name_;
};

muduo::ThreadLocal<Test> testObj1;
muduo::ThreadLocal<Test> testObj2;

void print()
{
    printf("tid = %d, obj1 %p name = %s\n",
          muduo::CurrentThread::tid(),
          &testObj1.value(),
          testObj1.value().name().c_str());

    printf("tid = %d, obj2 %p name = %s\n",
           muduo::CurrentThread::tid(),
           &testObj2.value(),
           testObj2.value().name().c_str());
}

void threadFunc()
{
    print();
    testObj1.value().setName("changed 1");
    testObj2.value().setName("changed 42");
}

int main()
{
    testObj1.value().setName("main one");   // value()函数中new新对象
    print();
    muduo::Thread t1(threadFunc);
    t1.start();
    t1.join();
    testObj2.value().setName("main two");
    print();

    // 一个线程可以隐式的退出，也可以显式的调用pthread_exit函数来退出。
    pthread_exit(0);

    return 0;
}



















