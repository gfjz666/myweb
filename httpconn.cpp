#include "httpconn.h"

const char *HttpConn::srcDir;
std::atomic<int> HttpConn::userCount;
bool HttpConn::isET;

HttpConn::HttpConn()
{
    fd_ = -1;
    addr_ = {0};
    isClose_ = true;
};

HttpConn::~HttpConn()
{
    closeHTTPConn();
};

void HttpConn::initHttpConn(int fd, const sockaddr_in &addr)
{
    assert(fd > 0);
    userCount++;
    addr_ = addr;
    fd_ = fd;
    writebuff.RetrieveAll(); // 清空读写指针
    readbuff.RetrieveAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, getIP(), getPort(), (int)userCount);
}

ssize_t HttpConn::readBuffer(int *saveErrno)
{
    ssize_t len = -1;
    do
    {
        len = readbuff.ReadFd(fd_, saveErrno);
        if (len <= 0)
        {
            break;
        }
    } while (isET);
    return len;
}

ssize_t HttpConn::writeBuffer(int *saveErrno)
{
    ssize_t len = -1;
    do
    {
        len = writev(fd_, iov_, iovCnt_);
        if (len <= 0)
        {
            *saveErrno = errno;
            break;
        }
        if (iov_[0].iov_len + iov_[1].iov_len == 0)
        {
            break;
        } /* 传输结束 */
        else if (static_cast<size_t>(len) > iov_[0].iov_len)
        {
            iov_[1].iov_base = (uint8_t *)iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len)
            {
                writebuff.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        }
        else
        {
            iov_[0].iov_base = (uint8_t *)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            writebuff.Retrieve(len);
        }
    } while (isET || writebytes() > 10240);
    return len;
}

void HttpConn::closeHTTPConn()
{
    response_.unmapFile_();
    if (isClose_ == false)
    {
        isClose_ = true;
        userCount--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, getIP(), getPort(), (int)userCount);
    }
}

bool HttpConn::handleHTTPConn()
{
    request_.Init();
    if (readbuff.ReadableBytes() <= 0)
        return false;
    else if (request_.parse(readbuff))
    {
        LOG_DEBUG("%s", request_.path().c_str());
        response_.Init(srcDir, request_.path(), request_.IsKeepAlive(), 200); // 解析成功返回200状态码
    }
    else
        response_.Init(srcDir, request_.path(), false, 400);

    response_.makeResponse(writebuff); // 生成 HTTP 响应，并将响应数据写入 writeBuff 缓冲区
    /* 响应头 */
    iov_[0].iov_base = const_cast<char *>(writebuff.Peek());
    iov_[0].iov_len = writebuff.ReadableBytes();
    iovCnt_ = 1;

    /* 文件 */
    if (response_.fileLen() > 0 && response_.file())
    {
        iov_[1].iov_base = response_.file();
        iov_[1].iov_len = response_.fileLen();
        iovCnt_ = 2;
    }
    LOG_DEBUG("filesize:%d, %d to %d", response_.fileLen(), iovCnt_, writebytes());
    return true;
}

const char *HttpConn::getIP() const
{
    return inet_ntoa(addr_.sin_addr);
}

int HttpConn::getPort() const
{
    return addr_.sin_port;
}

int HttpConn::getFd() const
{
    return fd_;
}

sockaddr_in HttpConn::getAddr() const
{
    return addr_;
}
