#ifndef THREADPOOL_H
#define THREADPOOL_H
#include<mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
class threadpool
{
private:
    /* data */
public:
    threadpool()=default;
    threadpool(threadpool&&)=default;
    explicit threadpool(int threadcount=8)
};

threadpool::threadpool(/* args */)
{
}

threadpool::~threadpool()
{
}




#endif