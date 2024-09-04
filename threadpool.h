#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
#include <assert.h>
class threadpool
{
private:
    struct Pool
    {
        std::mutex mtx_;
        std::condition_variable cond_;
        bool isClosed;
        std::queue<std::function<void()>> tasks; // 任务队列，函数类型为void()
    };
    std::shared_ptr<Pool> pool_;

public:
    threadpool() = default;
    threadpool(threadpool &&) = default;
    explicit threadpool(size_t threadcount = 8) : pool_(std::make_shared<Pool>())
    {
        assert(threadcount > 0);
        for (size_t i = 0; i < threadcount; i++)
        {
            std::thread([pool = pool_]
                        {
                    std::unique_lock<std::mutex> locker(pool->mtx_);
                    while(true) {
                        if(!pool->tasks.empty()) {
                            auto task = std::move(pool->tasks.front());
                            pool->tasks.pop();
                            locker.unlock();
                            task();
                            locker.lock();
                        } 
                        else if(pool->isClosed) break;
                        else pool->cond_.wait(locker);
                    } }).detach();
                
        }
    }
    ~threadpool()
    {
        if (static_cast<bool>(pool_))
        {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx_);
                pool_->isClosed = true;
            }
            pool_->cond_.notify_all();
        }
    }
    template <typename T>
    void AddTask(T &&task)
    {

        std::lock_guard<std::mutex> locker(pool_->mtx_);
        pool_->tasks.emplace(std::forward<T>(task));
        pool_->cond_.notify_one();
    }
};

#endif