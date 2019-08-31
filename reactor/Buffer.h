//
// Created by jxq on 19-8-31.
//

#ifndef MYMUDUO_BUFFER_H
#define MYMUDUO_BUFFER_H

#include <vector>
#include <unistd.h>
#include "../base/copyable.h"

namespace muduo
{

class Buffer : public muduo::copyable{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

private:
    char* begin()
    {
        return &*buffer_.begin();
    }

    const char* begin() const
    {
        return &*buffer_.begin();
    }

    void makeSpace(size_t len)
    {
        if ()
    }

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};

}


#endif //MYMUDUO_BUFFER_H
