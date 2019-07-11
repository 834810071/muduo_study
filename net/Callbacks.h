//
// Created by jxq on 19-7-10.
//

#ifndef MYMUDUO_CALLBACKS_H
#define MYMUDUO_CALLBACKS_H

#include "../base/Timestamp.h"

#include <functional>
#include <memory>

namespace muduo
{
    // 占位符
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    template<typename T>
    inline T* get_pointer(const std::shared_ptr<T>& ptr)
    {
        return ptr.get();
    }

    template<typename T>
    inline T* get_pointer(const std::unique_ptr<T>& ptr)
    {
        return ptr.get();
    }

    // Adapted from google-protobuf stubs/common.h
    // see License in muduo/base/Types.h
    template<typename To, typename From>
    inline ::std::shared_ptr<To> down_pointer_cast(const ::std::shared_ptr<From>& f)
    {
        if (false)
        {
            implicit_cast<From*, To*>(0);
        }

        #ifndef NDEBUG
            assert(f == NULL || dynamic_cast<To*>(get_pointer(f) != NULL));
        #endif

        return ::std::static_pointer_cast<To>(f);   // 实现shared_ptr类型转换
    }

    namespace net
    {
        // All client visible callbacks go here.

        class Buffer;
        class TcpConnection;
        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
        typedef std::function<void()> TimerCallback;
        typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
        typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
        typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
        typedef std::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

        // the data has been read to (buf, len)
        typedef std::function<void (const TcpConnectionPtr&,
                Buffer*,
                Timestamp)> MessageCallback;

        void defaultConnectionCallback(const TcpConnectionPtr& conn);
        void defaultMessageCallback(const TcpConnectionPtr& conn,
                                    Buffer* buffer,
                                    Timestamp receiveTime);

    }   // namespace net

}   // namespace muduo

#endif //MYMUDUO_CALLBACKS_H

















