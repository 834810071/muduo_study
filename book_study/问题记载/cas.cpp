//
// Created by jxq on 19-9-25.
//

#include <iostream>
#include <stdio.h>

using namespace std;

// 无锁的栈。
typedef ELEM int;
#define MAX (2048)

static ELEM Stack[MAX];
static int top = 0;

bool Push(ELEM val)
{
    int old_top;

    do
    {
        old_top = top;
        if (old_top >= MAX) return false;

        if (cas(&top, old_top, old_top + 1))
            break;

    }while(1);

    Stack[old_top] = val;

    return true;
}


bool Pop(ELEM& val)
{
    int old_top;
    do
    {
        old_top = top;

        if (old_top == 0) return false;

        val = Stack[old_top - 1];

        if (cas(&top, old_top, old_top - 1))
            break;

    } while(1);


    return true;
}