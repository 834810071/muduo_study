#include<iostream>
#include<memory>
#include<string.h>
#include<string>
#include <stdlib.h>
#include<algorithm>

class T
{
public:
    T(int i)
    {
        num = i;
    }
private:
    int num;
};

class B
{
public:
    explicit B(int i)
    {
        num = i;
    }
private:
    int num;
};

int main()
{
    T t = 12;
//    B b = 12;//不能隐身调用其构造函数
    B b(12);
    return 0;
}

