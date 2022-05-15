
#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>

explicit Buffer::Buffer(size_t initialSize = kInitialSize)
    : buffer_(initialSize + kCheapPrepend), /**/
      readerIndex_(kCheapPrepend),          /**/
      writerIndex_(kCheapPrepend)           /**/
{
}

// 从fd中读取数据
// fd默认工作在Level Trigger 模式 如果一次未把数据读完 下次epoll_wait还会继续返回此fd的可读事件
ssize_t Buffer::ReadFd(int fd, int *saveErrno)
{
    char extrabuf[65536]; //一次拷贝上来64k数据

    struct iovec vec[2];

    const size_t writable = WriteableBytes(); // buffer中可写的大小
    vec[0].iov_base = Begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[0].iov_base = extrabuf;
    vec[0].iov_len = sizeof(extrabuf);

    int iovcnt = writable < sizeof(extrabuf) ? 2 : 1;
    int n = readv(fd, vec, iovcnt);

    if (n < 0)
    {
        *saveErrno = errno;
    }
    else if (n < writable) // buffer足够存放
    {
        writerIndex_ += n;
    }
    else // buffer 不够存放 还有
    {
        writerIndex_ = buffer_.size();
        Append(extrabuf, n - writable);
    }

    return n;
}
