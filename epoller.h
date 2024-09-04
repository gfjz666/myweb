#pragma once
#include <sys/epoll.h> //epoll_ctl()
#include <fcntl.h>     // fcntl()
#include <unistd.h>    // close()
#include <assert.h>    // close()
#include <vector>
#include <errno.h>

// struct epoll_event {
//     uint32_t events;  // epoll 事件类型，包括可读，可写等
//     epoll_data_t data; // 用户数据，可以是一个指针或文件描述符等
// };
// EPOLLIN：表示对应的文件描述符上有数据可读
// EPOLLOUT：表示对应的文件描述符上可以写入数据
// EPOLLRDHUP：表示对端已经关闭连接，或者关闭了写操作端的写入
// EPOLLPRI：表示有紧急数据可读
// EPOLLERR：表示发生错误
// EPOLLHUP：表示文件描述符被挂起
// EPOLLET：表示将epoll设置为边缘触发模式
// EPOLLONESHOT：表示将事件设置为一次性事件

class Epoller
{
public:
    explicit Epoller(int maxEvent = 1024);
    ~Epoller();
    bool AddFd(int fd, uint32_t events);

    bool ModFd(int fd, uint32_t events);

    bool DelFd(int fd);

    int Wait(int timeoutMs = -1);

    int GetEventFd(size_t i) const;

    uint32_t GetEvents(size_t i) const;

private:
    int epollFd_;
    std::vector<struct epoll_event> events_;
};
