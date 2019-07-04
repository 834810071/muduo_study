//
// Created by jxq on 19-7-4.
//

#ifndef MYMUDUO_LOGSTREAM_H
#define MYMUDUO_LOGSTREAM_H

#include "noncopyable.h"
#include "StringPiece.h"
#include "Types.h"
#include <assert.h>
#include <string.h>

namespace muduo
{
    namespace detail {
        // 小的缓冲区大小
        const int kSmallBuffer = 4000;
        // 大的缓冲区大小
        const int kLargeBuffer = 4000 * 1000;

        /*
         * 固定大小的buffer
         */
        template<int SIZE>
        class FixedBuffer : noncopyable
        {
        public:
            FixedBuffer()
                : cur_(data_)
            {
                setCookie(cookieStart);
            }

            ~FixedBuffer()
            {
                setCookie(cookieEnd);
            }

            // 添加数据
            void append(const char* /*restrict*/ buf, size_t len)
            {
                // FIXME: append partially
                if (implicit_cast<size_t >(avail()) > len)  // 如果可用数据足够，就拷贝过去，同时移动当前指针。
                {
                    memcpy(cur_, buf, len);
                    cur_ += len;
                }
            }

        private:
            const char* end() const
            {
                return data_ + sizeof data_;
            }

            // Must be outline function for cookies.
            static void cookieStart();
            static void cookieEnd();

            void (*cookie_)();  // 回调函数
            char data_[SIZE];   // 缓冲区
            char* cur_;         // 指针，指向缓冲区读写的位置
        };
    }
}


#endif //MYMUDUO_LOGSTREAM_H
























