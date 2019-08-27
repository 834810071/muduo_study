//
// Created by jxq on 19-8-26.
//

#ifndef MYMUDUO_CALLBACKS_H
#define MYMUDUO_CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "../../base/Timestamp.h"

namespace muduo
{
    typedef boost::function<void()> TimerCallback;
}

#endif //MYMUDUO_CALLBACKS_H
