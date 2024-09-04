#pragma once
#include <unordered_map>
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "log.h"
#include "epoller.h"
#include "httpconn.h"
#include "sqlconnRAII.h"
#include "sqlconnpool.h"
#include "threadpool.h"
#include "heaptimer.h"

class WebServer
{
public:
    //构造函数用于初始化服务器的各种参数，包括端口号、触发模式、超时时间、数据库连接信息、线程池大小、日志选项等
    WebServer(int port, int trigMode, int timeoutMS, bool OptLinger,
              int sqlPort, const char *sqlUser, const char *sqlPwd,
              const char *dbName, int connPoolNum, int threadNum,
              bool openLog, int logLevel, int logQueSize);
    ~WebServer();
    void Start();

private:
    bool InitSocket_();
    void InitEventMode_(int trigMode);
    void AddClient_(int fd, sockaddr_in addr);

    void DealListen_();
    void DealWrite_(HttpConn *client);
    void DealRead_(HttpConn *client);

    void SendError_(int fd, const char *info);
    void ExtentTime_(HttpConn *client);//延长客户端连接的超时时间
    void CloseConn_(HttpConn *client);

    void OnRead_(HttpConn *client);
    void OnWrite_(HttpConn *client);
    void OnProcess(HttpConn *client);

    static const int MAX_FD = 65536;

    static int SetFdNonblock(int fd);

    int port_;
    bool openLinger_;
    int timeoutMS_; /* 毫秒MS */
    bool isClose_;
    int listenFd_; // 服务器监听套接字的文件描述符。
    char *srcDir_;

    uint32_t listenEvent_;
    uint32_t connEvent_;

    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<threadpool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConn> users_;
};
