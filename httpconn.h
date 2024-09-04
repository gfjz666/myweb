#pragma once
#include <sys/types.h>
#include <sys/uio.h>   // readv/writev
#include <arpa/inet.h> // sockaddr_in
#include <stdlib.h>    // atoi()
#include <errno.h>

#include "log.h"
#include "buffer.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "sqlconnRAII.h"

class HttpConn
{
public:
    HttpConn();
    ~HttpConn();
    void initHttpConn(int socketFd, const sockaddr_in &addr);

    // 每个连接中定义的对缓冲区的读写接口
    ssize_t readBuffer(int *saveErrno);
    ssize_t writeBuffer(int *saveErrno);

    // 关闭HTTP连接的接口
    void closeHTTPConn();
    // 定义处理该HTTP连接的接口，主要分为request的解析和response的生成
    bool handleHTTPConn();

    // 其他方法
    const char *getIP() const;
    int getPort() const;
    int getFd() const;
    sockaddr_in getAddr() const;
    int writebytes() { return iov_[1].iov_len + iov_[0].iov_len; }
    bool IsKeepAlive() const { return request_.IsKeepAlive(); }
    static bool isET;
    static const char *srcDir;
    static std::atomic<int> userCount;

private:
    int fd_; // HTTP连接对应的描述符
    struct sockaddr_in addr_;
    bool isClose_; // 标记是否关闭连接

    int iovCnt_;
    struct iovec iov_[2];

    buffer readbuff;
    buffer writebuff;
    HttpRequest request_;
    HTTPresponse response_;
};
