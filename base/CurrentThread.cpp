//
// Created by jxq on 19-6-20.
//

#include "CurrentThread.h"

#include <cxxabi.h>
#include <execinfo.h>   // 在头文件"execinfo.h"中声明了三个函数用于获取当前线程的函数调用堆栈
#include <stdlib.h>

namespace muduo
{
    namespace CurrentThread
    {
        __thread int t_cachedTid = 0;
        __thread char t_tidString[32];
        __thread int t_tidStringLength = 6;
        __thread const char* t_threadName = "unknown";
        // static_assert这个关键字，用来做编译期间的断言，因此叫做静态断言。
        static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

        string stackTrace(bool demangle)    // demangle 解构，还原函数
        {
            string stack;
            const int max_frames = 200;
            void* frame[max_frames];
            // Function: int backtrace(void **buffer,int size)
            //
            // 该函数用与获取当前线程的调用堆栈,获取的信息将会被存放在buffer中,它是一个指针列表。参数 size 用来指定buffer中可以保存多少个void* 元素。
            // 函数返回值是实际获取的指针个数,最大不超过size大小，在buffer中的指针实际是从堆栈中获取的返回地址,每一个堆栈框架有一个返回地址，
            //  注意某些编译器的优化选项对获取正确的调用堆栈有干扰,另外内联函数没有堆栈框架;删除框架指针也会使无法正确解析堆栈内容。

            // 栈回溯，保存各个栈帧的地址
            int nptrs = ::backtrace(frame, max_frames); // 从堆栈中获取的返回地址

            // Function: char ** backtrace_symbols (void *const *buffer, int size)
            //
            //backtrace_symbols将从backtrace函数获取的信息转化为一个字符串数组.
            // 参数buffer应该是从backtrace函数获取的数组指针,size是该数组中的元素个数(backtrace的返回值)。
            // 函数返回值是一个指向字符串数组的指针,它的大小同buffer相同.每个字符串包含了一个相对于buffer中对应元素的可打印信息.它包括函数名，函数的偏移地址,和实际的返回地址。

            // 根据地址，转成相应的函数符号
            char** strings = ::backtrace_symbols(frame, nptrs); // 一个指向字符串数组的指针

            if (strings)
            {
                size_t len = 256;
                char* demangled = demangle ? static_cast<char*>(::malloc(len)) : nullptr;
                // 遍历堆栈中的函数
                for (int i = 1; i < nptrs; ++i)
                {
                    if (demangle)
                    {
                        char* left_par = nullptr;   // 左括号
                        char* plus = nullptr;
                        // 查看该函数名是否包含'(' 和 '+'
                        for (char* p = strings[i]; *p; ++p)
                        {
                            if (*p == '(')
                            {
                                left_par = p;
                            }
                            else if (*p == '+')
                            {
                                plus = p;
                            }
                        }

                        if (left_par && plus)
                        {
                            *plus = '\0';
                            int status = 0;
                            char* ret = abi::__cxa_demangle(left_par+1, demangled, &len, &status);  // 类名
                            *plus = '+';
                            // 0: The demangling operation succeeded.
                            if (status == 0)
                            {
                                demangled = ret; // ret could be realloc()
                                stack.append(strings[i], left_par+1);   // append : 追加  参数
                                stack.append(demangled);    // 函数名
                                stack.append(plus);
                                stack.push_back('\n');
                                continue;
                            }
                        }
                    }

                    // Fallback to mangled names;
                    stack.append(strings[i]);
                    stack.push_back('\n');
                }
                free(demangled);
                free(strings);
            }
            return stack;
        }
    }   // namespace CurrentThread;
}   // namespace muduo;
























