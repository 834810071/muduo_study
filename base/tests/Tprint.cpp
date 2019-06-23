//
// Created by jxq on 19-6-15.
//
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#include <iostream>

#define SENDSIGTIME 10

using namespace std;

class A
{
public:
    A(int n)
        : n_(n)
    {

    }

    ~A()
    {

    }

    int getN_() const {
        return n_;
    }

    void setN_(int n_) {
        A::n_ = n_;
    }

    static int getNum_() {
        return num_;
    }

    static void setNum_(int num_) {
        A::num_ = num_;
    }

private:
    int n_;
    static int num_;
};
int A::num_ = 0;

int main(void)
{
    A* a = new A(2);
    a->setNum_(a->getNum_() + 1);
    cout << a->getNum_() << endl;
    return 0;
}


