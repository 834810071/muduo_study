//
// Created by jxq on 19-7-2.
//
#include "../ThreadLocalSingleton_test.h"
#include "../CurrentThread.h"
#include "../Thread.h"

#include <stdio.h>
#include <unistd.h>

class Test : muduo::noncopyable
{
public:
    Test()
    {
        printf("tid = %d, constructing %p\n", muduo::CurrentThread::tid(), this);
    }

    ~Test()
    {
        printf("tid = %d, destructing %p %s\n", muduo::CurrentThread::tid(), this, name_.c_str());
    }

    const muduo::string& name() const
    {
        return name_;
    }

    void setName(const muduo::string& n)
    {
        name_ = n;
    }

private:
    muduo::string name_;
};

void threadFunc(const char* changeTo)
{
    printf("tid = %d, %p name = %s\n",
            muduo::CurrentThread::tid(),
            &muduo::ThreadLocalSingleton<Test>::instance(),
           muduo::ThreadLocalSingleton<Test>::instance().name().c_str());
    muduo::ThreadLocalSingleton<Test>::instance().setName(changeTo);
    // sleep(2);
    printf("tid = %d, %p name = %s\n",
           muduo::CurrentThread::tid(),
           &muduo::ThreadLocalSingleton<Test>::instance(),
           muduo::ThreadLocalSingleton<Test>::instance().name().c_str());
}

int main()
{
    muduo::ThreadLocalSingleton<Test>::instance().setName("main one");
    muduo::Thread t1(bind(threadFunc, "thread1"));
    muduo::Thread t2(bind(threadFunc, "thread2"));
    t1.start();
    t2.start();
    t1.join();
    printf("tid = %d, %p name = %s\n",
           muduo::CurrentThread::tid(),
           &muduo::ThreadLocalSingleton<Test>::instance(),
           muduo::ThreadLocalSingleton<Test>::instance().name().c_str());
    t2.join();

    pthread_exit(0);
    return 0;
}