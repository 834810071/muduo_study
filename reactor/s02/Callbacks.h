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

    // All client visible callbacks go here.
    class TcpConnection;
    typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

    typedef boost::function<void()> TimerCallback;
    typedef boost::function<void (const TcpConnectionPtr&)> ConnectionCallback;
    typedef boost::function<void (const TcpConnectionPtr&,
                                  const char* data,
                                  ssize_t len)> MessageCallback;
    typedef boost::function<void (const TcpConnectionPtr&)> CloseCallback;
}

#endif //MYMUDUO_CALLBACKS_H
