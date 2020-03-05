//
// Created by jxq on 19-7-4.
//

#include "LogStream.h"

#include <algorithm>
#include <limits>
#include <type_traits>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

// 使用PRIu32进行格式化的输出的时候需要引用：#include <inttypes.h>
// 需要同时增加编译宏：__STDC_FORMAT_MACRS
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

using namespace muduo;
using namespace muduo::detail;

// TODO:: better itoa.
#if defined(__clang__)
// #pragma用于指示编译器完成一些特定的动作
#pragma clang diagnostic ignored "-Wtautological-compare"
#else
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

namespace muduo
{
    namespace detail
    {
        const char digits[] = "9876543210123456789";
        const char* zero = digits + 9;
        static_assert(sizeof(digits) == 20, "wrong number of digits");

        const char digitsHex[] = "0123456789ABCDEF";
        static_assert(sizeof(digitsHex) == 17, "wrong number of digitsHex");

        // 将整数转换为字符串
        template<typename T>
        size_t convert(char buf[], T value)
        {
            T i = value;
            char* p = buf;

            do
            {
                // lsd : last digit,最后一位数字
                int lsd = static_cast<int>(i % 10);
                i /= 10;
                *p++ = zero[lsd];
            } while (i != 0);

            if (value < 0)
            {
                *p++ = '-';
            }
            *p = '\0';
            std::reverse(buf, p);

            return p - buf;
        }

        // uintptr_t : unsigned long int
        size_t convertHex(char buf[], uintptr_t value)
        {
            uintptr_t i = value;
            char* p = buf;

            do
            {
                int lsd = static_cast<int>(i % 16);
                i /= 16;
                *p++ = digitsHex[lsd];
            } while (i != 0);

            *p = '\0';
            std::reverse(buf, p);

            return p - buf;
        }

        template class FixedBuffer<kSmallBuffer>;   // 模版类
        template class FixedBuffer<kLargeBuffer>;



    }   // namespace detail;

    /*
     Format a number with 5 characters, including SI units.
     [0,     999]
     [1.00k, 999k]
     [1.00M, 999M]
     [1.00G, 999G]
     [1.00T, 999T]
     [1.00P, 999P]
     [1.00E, inf)
    */
    std::string formatSI(int64_t s)
    {
        double n = static_cast<double>(s);
        char buf[64];
        if (s < 1000)
            snprintf(buf, sizeof(buf), "%" PRId64, s);
        else if (s < 9995)
            snprintf(buf, sizeof(buf), "%.2fk", n/1e3);
        else if (s < 99950)
            snprintf(buf, sizeof(buf), "%.1fk", n/1e3);
        else if (s < 999500)
            snprintf(buf, sizeof(buf), "%.0fk", n/1e3);
        else if (s < 9995000)
            snprintf(buf, sizeof(buf), "%.2fM", n/1e6);
        else if (s < 99950000)
            snprintf(buf, sizeof(buf), "%.1fM", n/1e6);
        else if (s < 999500000)
            snprintf(buf, sizeof(buf), "%.0fM", n/1e6);
        else if (s < 9995000000)
            snprintf(buf, sizeof(buf), "%.2fG", n/1e9);
        else if (s < 99950000000)
            snprintf(buf, sizeof(buf), "%.1fG", n/1e9);
        else if (s < 999500000000)
            snprintf(buf, sizeof(buf), "%.0fG", n/1e9);
        else if (s < 9995000000000)
            snprintf(buf, sizeof(buf), "%.2fT", n/1e12);
        else if (s < 99950000000000)
            snprintf(buf, sizeof(buf), "%.1fT", n/1e12);
        else if (s < 999500000000000)
            snprintf(buf, sizeof(buf), "%.0fT", n/1e12);
        else if (s < 9995000000000000)
            snprintf(buf, sizeof(buf), "%.2fP", n/1e15);
        else if (s < 99950000000000000)
            snprintf(buf, sizeof(buf), "%.1fP", n/1e15);
        else if (s < 999500000000000000)
            snprintf(buf, sizeof(buf), "%.0fP", n/1e15);
        else
            snprintf(buf, sizeof(buf), "%.2fE", n/1e18);
        return buf;
    }

    /*
     [0, 1023]
     [1.00Ki, 9.99Ki]
     [10.0Ki, 99.9Ki]
     [ 100Ki, 1023Ki]
     [1.00Mi, 9.99Mi]
    */
    std::string formatIEC(int64_t s)
    {
        double n = static_cast<double>(s);
        char buf[64];
        const double Ki = 1024.0;
        const double Mi = Ki * 1024.0;
        const double Gi = Mi * 1024.0;
        const double Ti = Gi * 1024.0;
        const double Pi = Ti * 1024.0;
        const double Ei = Pi * 1024.0;

        if (n < Ki)
            snprintf(buf, sizeof buf, "%" PRId64, s);
        else if (n < Ki*9.995)
            snprintf(buf, sizeof buf, "%.2fKi", n / Ki);
        else if (n < Ki*99.95)
            snprintf(buf, sizeof buf, "%.1fKi", n / Ki);
        else if (n < Ki*1023.5)
            snprintf(buf, sizeof buf, "%.0fKi", n / Ki);

        else if (n < Mi*9.995)
            snprintf(buf, sizeof buf, "%.2fMi", n / Mi);
        else if (n < Mi*99.95)
            snprintf(buf, sizeof buf, "%.1fMi", n / Mi);
        else if (n < Mi*1023.5)
            snprintf(buf, sizeof buf, "%.0fMi", n / Mi);

        else if (n < Gi*9.995)
            snprintf(buf, sizeof buf, "%.2fGi", n / Gi);
        else if (n < Gi*99.95)
            snprintf(buf, sizeof buf, "%.1fGi", n / Gi);
        else if (n < Gi*1023.5)
            snprintf(buf, sizeof buf, "%.0fGi", n / Gi);

        else if (n < Ti*9.995)
            snprintf(buf, sizeof buf, "%.2fTi", n / Ti);
        else if (n < Ti*99.95)
            snprintf(buf, sizeof buf, "%.1fTi", n / Ti);
        else if (n < Ti*1023.5)
            snprintf(buf, sizeof buf, "%.0fTi", n / Ti);

        else if (n < Pi*9.995)
            snprintf(buf, sizeof buf, "%.2fPi", n / Pi);
        else if (n < Pi*99.95)
            snprintf(buf, sizeof buf, "%.1fPi", n / Pi);
        else if (n < Pi*1023.5)
            snprintf(buf, sizeof buf, "%.0fPi", n / Pi);

        else if (n < Ei*9.995)
            snprintf(buf, sizeof buf, "%.2fEi", n / Ei );
        else
            snprintf(buf, sizeof buf, "%.1fEi", n / Ei );
        return buf;
    }
}   // namespace muduo

template<int SIZE> // 使用非类型模版参数
const char* FixedBuffer<SIZE>::debugString()
{
    *cur_ = '\0';   // 加个\把缓冲区作为字符串
    return data_;
}

// 这两个函数的目的不明
template<int SIZE>
void FixedBuffer<SIZE>::cookieStart()
{

}

template <int SIZE>
void FixedBuffer<SIZE>::cookieEnd()
{

}

// 静态检查，用于检查一些类型的大小
void LogStream::staticCheck()
{
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10,         // 15
            "kMaxNumericSize is larger enough");
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10,    // 18
                  "kMaxNumericSize is large enough");
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10,           // 18
                  "kMaxNumericSize is large enough");
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10,      // 18
                  "kMaxNumericSize is large enough");
}

template<typename T>
void LogStream::formatInteger(T v)
{
    if (buffer_.avail() >= kMaxNumericSize)
    {
        size_t len = convert(buffer_.current(), v); // 整数转换为字符串，返回长度
        buffer_.add(len);   // 修改cur_指针位置
    }
}

// 下面这些<<最终都要调用formatIneteger()将字符串转换成整数
LogStream& LogStream::operator<<(short v)
{
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
    formatInteger(v);
    return *this;
}

// 将普通的数据转换成16进制存储
LogStream& LogStream::operator<<(const void* p)
{
    // 在C语言中，任何类型的指针都可以转换为void *类型，并且在将它转换回原来的类型时不会丢失信息
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    // 保证了，任意的基础类型都能够存放在缓冲区中而不至于溢出
    if (buffer_.avail() >= kMaxNumericSize)
    {
        char* buf = buffer_.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = convertHex(buf+2, v);
        buffer_.add(len+2);
    }
    return *this;
}

// FIXME:: replace this with Grisu3 by Florian Loitsch
LogStream& LogStream::operator<<(double v)
{
    if (buffer_.avail() >= kMaxNumericSize)
    {
        int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
        buffer_.add(len);
    }
    return *this;
}

LogStream& LogStream::operator<<(long double v) {
    if (buffer().avail() >= kMaxNumericSize) {
        int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12Lg", v);
        buffer_.add(len);
    }
    return *this;
}

template<typename T>
Fmt::Fmt(const char *fmt, T val)
{
    // 断言是算术类型
    static_assert(std::is_arithmetic<T>::value == true, "Must be arithmetic type");

    // 格式化到缓冲区
    length_ = snprintf(buf_, sizeof(buf_), fmt, val);
    assert(static_cast<size_t>(length_) < sizeof buf_);
}

// Explicit instantiations
// 模板的特化，只支持这么多类型
template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);



















