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
    return BeginPtr_() + readpos;
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
    // write指针向后移动
    // 将Fd写入到readable区
    writepos += len;
}

void buffer::Retrieve(size_t len)
{
    // read指针向后移动
    // 从readable区域读出数据
    readpos += len;
}

void buffer::RetrieveUntil(const char *end)
{
    // read指针移动到end
    assert(Peek() <= end);
    Retrieve(end - Peek());
}

void buffer::RetrieveAll()
{
    // 取出所有数据，buffer归零，读写下标归零,在别的函数中会用到
    // bzero() 能够将内存块（字符串）的前n个字节清零
    bzero(&buffer_[0], buffer_.size());
    readpos = 0;
    writepos = 0;
}

std::string buffer::RetrieveAllTostr()
{
    // 取出剩余可读的str
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

const char *buffer::BeginWriteConst() const
{
    // 写指针的位置
    return BeginPtr_()+writepos;
}

char *buffer::BeginWrite()
{
    return &buffer_[writepos];
}

// 添加str到缓冲区

void buffer::append(const char *str, size_t len)
{
    assert(str);
    EnsureWritable(len);
    std::copy(str, str + len, BeginWrite()); // 将str放到写下标开始的地方
    HasWritten(len);                       // 移动写下标
}

void buffer::append(const std::string &str)
{
    append(str.c_str(), str.size());
}

void buffer::append(const void *data, size_t len)
{
    assert(data);
    append(static_cast<const char*>(data), len);
}

void buffer::append(const buffer &buff)
{
    append(buff.Peek(), buff.ReadableBytes());
}

ssize_t buffer::ReadFd(int fd, int *Errno)
{
    //fd读到缓冲区
    char buff[65535]; // 栈区
    struct iovec iov[2];
    const size_t writeable = WritableBytes(); // 先记录能写多少
    // 分散读， 保证数据全部读完
    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = writeable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if (len < 0)
    {
        *Errno = errno;
    }
    else if (static_cast<size_t>(len) <= writeable)
    {                    // 若len小于writable，说明写区可以容纳len
        writepos += len; // 直接移动写下标
    }
    else
    {
        writepos = buffer_.size();                          // 写区写满了,下标移到最后
        append(buff, len - writeable); // 剩余的长度
    }
    return len;
}

ssize_t buffer::WriteFd(int fd, int *Errno)
{
    //写进fd
    ssize_t len = write(fd, Peek(), ReadableBytes());
    if (len < 0)
    {
        *Errno = errno;
        return len;
    }
    Retrieve(len);
    return len;
}

const char *buffer::BeginPtr_() const
{
    // 缓冲区头部地址
    return &buffer_[0];
}

char *buffer::BeginPtr_()
{
    // 缓冲区头部地址
    return &buffer_[0];
}

void buffer::MakeSpace_(size_t len)
{
    // 实现空间扩容
    if (WritableBytes() + PreendableBytes() < len)
    {
        // 实际可用空间小于需要写入的空间
        buffer_.resize(writepos + len + 1);
    }
    else
    {
        // 内存挪动
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readpos, BeginPtr_() + writepos, BeginPtr_());
        readpos = 0;
        writepos = readable;
        assert(readable == ReadableBytes());
    }
}
