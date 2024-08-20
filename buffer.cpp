#include "buffer.h"

/*
 * 读写下标初始化,buffer数组初始化
 * 缓冲区按顺序为：预留空间、读空间、写空间
 * 指针分布为：0，读指针、写指针
 */

buffer::buffer(int initBuffsize) : buffer_(initBuffsize), readpos(0), writepos(0) {}

size_t buffer::WritableBytes() const
{
    // 可写空间
    return buffer_.size() - writepos;
}
size_t buffer::ReadableBytes() const
{
    // 可读空间
    return writepos - readpos;
}
size_t buffer::PreendableBytes() const
{
    // 可预留空间
    return readpos;
}

const char *buffer::Peek() const
{
    return &buffer_[readpos];
}

void buffer::EnsureWritable(size_t len)
{
    // 确保可写的长度
    if (len > WritableBytes())
        MakeSpace_(len);

    assert(len <= WritableBytes());
}

void buffer::HasWritten(size_t len)
{
}

void buffer::Retrieve(size_t len)
{
}

void buffer::RetrieveUntil(const char *end)
{
}

const char *buffer::Beginptr() const
{
    return &buffer_[0];
}

char *buffer::BeginPtr_()
{
    return &buffer_[0];
}

void buffer::MakeSpace_(size_t len)
{
}
