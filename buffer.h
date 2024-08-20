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

    const char *Beginptr() const;
    char *BeginPtr_();
    void MakeSpace_(size_t len);

public:
    buffer(int initBuffsize = 1024);
    ~buffer() = default;

    size_t WritableBytes() const;
    size_t ReadableBytes() const;
    size_t PreendableBytes() const;

    const char *Peek() const;
    void EnsureWritable(size_t len); 
    void HasWritten(size_t len);

    void Retrieve(size_t len);
    void RetrieveUntil(const char *end);

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void append(const std::string &str);
    void append(const char*str, size_t len);
    


};
