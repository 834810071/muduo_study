//
// Created by jxq on 19-6-20.
//

#include "Exception.h"
#include "CurrentThread.h"

namespace muduo
{
    Exception::Exception(std::string msg)
        : message_(std::move(msg)),  //std::move是将对象的状态或者所有权从一个对象转移到另一个对象，只是转移，没有内存的搬迁或者内存拷贝。
        stack_(CurrentThread::stackTrace(/*demangle=*/false))
    {

    }
}   // namespace muduo
