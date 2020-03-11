//
// Created by jxq on 19-6-21.
//

#include "../CurrentThread.h"
#include "../Exception.h"

#include <functional>
#include <vector>
#include <stdio.h>

using namespace std;

class Bar
{
public:
    void test(vector<string> name = {})
    {
        printf("Stack:\n%s\n", muduo::CurrentThread::stackTrace(true).c_str());
        []
        {
            printf("Stack inside lambda:\n%s\n", muduo::CurrentThread::stackTrace(true).c_str());
        }();

        function<void()> func([] {
            printf("Stack inside std::function:\n%s\n", muduo::CurrentThread::stackTrace(true).c_str());
        });

        func();
        func = std::bind(&Bar::callback, this);
        func();

        throw muduo::Exception("oops");
    }

private:
    void callback()
    {
        printf("Stack inside std::bind:\n%s\n", muduo::CurrentThread::stackTrace(true).c_str());
    }
};

void foo()
{
    Bar b;
    b.test();
}

int main(int argc, char** argv)
{
    try
    {
        foo();
    }
    catch (const muduo::Exception& ex)
    {
        printf("reson: %s\n", ex.what());
        printf("stack trace:\n%s\n", ex.stackTrace());
    }
    return 0;
}