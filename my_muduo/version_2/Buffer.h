#pragma once

#include <vector>
#include <string>

#include <algorithm>

class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    Buffer(size_t initialSize = kInitialSize);

    size_t ReadableBytes() const { return writerIndex_ - readerIndex_; }
    //返回可写的数据
    size_t WriteableBytes() const { return buffer_.size() - writerIndex_; }

    size_t PrependableBytes() const { return readerIndex_; }

    //返回缓冲区中可读数据的起始地址
    const char *Peek() const { return Begin() + readerIndex_; }

    //用户调用 string << buffer
    void Retrieve(size_t len)
    {
        if (len < ReadableBytes())
        {
            //可读数据未一次全部读取
            readerIndex_ += len;
        }
        else
        {
            //可读数据已被全部读取将下标复位
            RetrieveAll();
        }
    }

    //将 readerIndex_  writerIndex_ 全部复位
    void RetrieveAll()
    {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    //将所有已读到的数据转成string返回
    std::string RetrieveAllAsString()
    {
        return RetrieveAsString(ReadableBytes()); //
    }

    std::string RetrieveAsString(size_t len)
    {
        std::string ret(Peek(), len);
        Retrieve(len); //可读数据已经被取出 对缓冲区进行复位
        return ret;
    }

    //检查写缓冲区是否需要扩容
    void EnsureWriteableBytes(size_t len)
    {
        if (WriteableBytes() < len)
        {
            MakeSpace(len); //可写区需要扩容
        }
    }

    void MakeSpace(size_t len)
    {
        // 如果数据向前挪动还是无法腾出足够的空间就需要扩容
        if (WriteableBytes() + PrependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        //将剩余可读数据向前挪动 腾出足够空间
        else
        {
            size_t readable = ReadableBytes();
            std::copy(Begin() + readerIndex_,
                      Begin() + writerIndex_,
                      Begin() + kCheapPrepend);

            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    //将data拷贝len个字节到buffer的写缓冲区
    void Append(const char *data, size_t len)
    {
        EnsureWriteableBytes(len);
        std::copy(data, data + len, BeginWrite());
        //更新写指针
        writerIndex_ += len;
    }

    //从fd中读取数据
    ssize_t ReadFd(int fd, int *saveErrno);
    //通过fd发送数据
    ssize_t WriteFd(int fd, int *saveErrno);

private:
    char *Begin() { return buffer_.data(); }
    const char *Begin() const { return buffer_.data(); }

    //返回写缓冲区的起始地址
    char *BeginWrite() { return buffer_.data() + writerIndex_; }
    const char *BeginWrite() const { return buffer_.data() + writerIndex_; }

private:
    std::vector<char> buffer_;
    size_t writerIndex_;
    size_t readerIndex_;
};