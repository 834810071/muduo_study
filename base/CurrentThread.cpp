//
// Created by jxq on 19-6-20.
//

#include "CurrentThread.h"

#include <cxxabi.h>
#include <execinfo.h>
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

        string stackTrace(bool demangle)
        {
            string stack;
            const int max_frames = 200;
            void* frame[max_frames];
            int nptrs = ::backtrace(frame, max_frames);
            char** strings = ::backtrace_symbols(frame, nptrs);
        }
    }
}