#pragma once
#include <iostream>
#include <vector>
#include <sys/uio.h> //readv
#include <assert.h>  //readv
#include <unistd.h>  //write
#include <atomic>
#include <cstring>

class buffer
{
private:
    std::vector<char> buffer_;    // 缓冲区
    std::atomic<size_t> readpos;  // 读下标
    std::atomic<size_t> writepos; // 写下标

    const char *BeginPtr_() const;
    char *BeginPtr_();
    void MakeSpace_(size_t len);

public:
    buffer(int initBuffsize = 1024);
    ~buffer() = default;

    // 缓冲区可以读写、预留空间的字节数
    size_t WritableBytes() const;
    size_t ReadableBytes() const;
    size_t PreendableBytes() const;

    // 得到预留空间的尾指针
    const char *Peek() const;
    // 确保可写，否则扩容
    void EnsureWritable(size_t len);
    void HasWritten(size_t len);

    void Retrieve(size_t len);
    void RetrieveUntil(const char *end);

    void RetrieveAll();
    std::string RetrieveAllTostr();

    const char *BeginWriteConst() const;
    char *BeginWrite();

    // 向缓冲区写入数据,重载append
    void append(const std::string &str);
    void append(const char *str, size_t len);
    void append(const void *data, size_t len);
    void append(const buffer &buff);

    // 从socket通信的对端，接收数据到缓冲区
    ssize_t ReadFd(int fd, int *Errno);
    // 将缓冲区中的数据，发送到socket通信的对端
    ssize_t WriteFd(int fd, int *Errno);
};
