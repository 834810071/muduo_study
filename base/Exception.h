//
// Created by jxq on 19-6-20.
//

#ifndef MYMUDUO_EXCEPTION_H
#define MYMUDUO_EXCEPTION_H

#include "Types.h"
#include <exception>

namespace muduo
{
    class Exception : public std::exception
    {
    public:
        Exception(string what);
        // noexcept 该关键字告诉编译器，函数中不会发生异常,这有利于编译器对程序做更多的优化
        ~Exception() noexcept override = default;   // 虚函数需要写override

        const char* what() const noexcept override
        {
            return message_.c_str();
        }

        const char* stackTrace() const noexcept
        {
            return stack_.c_str();
        }

    private:
        string message_;
        string stack_;
    };
}


#endif //MYMUDUO_EXCEPTION_H
