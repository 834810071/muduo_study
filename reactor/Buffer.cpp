//
// Created by jxq on 19-8-31.
//

#include <sys/uio.h>
#include <errno.h>
#include "Buffer.h"
#include "../base/Types.h"

using namespace muduo;

ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writableBytes();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    const ssize_t n = readv(fd, vec, 2);
    if (n < 0)
    {
        *savedErrno = errno;
    }
        // 读取的数据小于buffer_长度
    else if (implicit_cast<size_t >(n) < writable)  // implicit_cast 只能执行向上转换
    {
        writerIndex_ += n;
    }
    else    // 读取的数据大于buffer_长度
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}