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

    }
}

