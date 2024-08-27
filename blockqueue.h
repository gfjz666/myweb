#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H
#include <deque>
#include <condition_variable>
#include <mutex>
#include <sys/time.h>
using namespace std;
/*采用deque实现阻塞队列
  其内部搭配锁条件变量实现
  生产者防止队列空，消费者防止队列为满
*/
// 模板类
template <typename T>
class BlockQueue
{
public:
    explicit BlockQueue(size_t maxsize = 1000); // 防止隐构造
    ~BlockQueue();
    bool empty();
    bool full();
    void push_back(const T &item);
    void push_front(const T &item);
    bool pop(T &item);              // 弹出的任务放入item
    bool pop(T &item, int timeout); // 等待时间
    void clear();
    T front();
    T back();
    size_t capacity();
    size_t size();
    void flush();
    void close();

private:
    deque<T> deq_; // 阻塞队列
    mutex mtx_;    // 互斥锁
    bool isclose;  // 标志队列是否关闭
    size_t capacity_;
    condition_variable condConsumer_; // 消费者条件变量
    condition_variable condProducer_; // 消费者条件变量
};
template <typename T>
BlockQueue<T>::BlockQueue(size_t maxsize) : capacity_(maxsize)
{
    assert(maxsize > 0);
    isclose = false;
}
template <typename T>
BlockQueue<T>::~BlockQueue()
{
    close();
}
template <typename T>
bool BlockQueue<T>::empty()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.empty();
}
template <typename T>
bool BlockQueue<T>::full()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size() >= capacity_;
}
template <typename T>
void BlockQueue<T>::push_back(const T &item)
{
    unique_lock<mutex> locker(mtx_);
    while (deq_.size() >= capacity_) // 如果队列满了需要阻塞等待唤醒
    {
        condProducer_.wait(locker);
    }
    deq_.push_back(item);
    condConsumer_.notify_one();
}
template <typename T>
void BlockQueue<T>::push_front(const T &item)
{
    unique_lock<mutex> locker(mtx_);
    while (deq_.size() >= capacity_) // 如果队列满了需要阻塞等待唤醒
    {
        condProducer_.wait(locker);
    }
    deq_.push_front(item);
    condConsumer_.notify_one();
}
template <typename T>
bool BlockQueue<T>::pop(T &item)
{
    unique_lock<mutex> locker(mtx_);
    while (deq_.empty())
        condConsumer_.wait(locker);
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}
template <typename T>
bool BlockQueue<T>::pop(T &item, int timeout)
{
    // 带有超时机制的pop
    unique_lock<mutex> locker(mtx_);
    while (deq_.empty())
    {
        // 指定等待时间
        if (condConsumer_.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout)
            return false;
        if (isclose)
            return false;
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}
template <typename T>
void BlockQueue<T>::close()
{
    // 关闭阻塞队列
    std::lock_guard<std::mutex> locker(mtx_); // 上锁
    clear();
    isclose = true;
    condConsumer_.notify_all(); // 唤醒所有消费者
    condProducer_.notify_all(); // 唤醒所有生产者
}
template <typename T>
void BlockQueue<T>::flush()
{
    // 唤醒一个消费者
    condConsumer_.notify_one();
}
template <typename T>
void BlockQueue<T>::clear()
{
    // 清空队列
    std::lock_guard<std::mutex> locker(mtx_);
    deq_.clear();
}
template <typename T>
T BlockQueue<T>::front()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.front();
}
template <typename T>
T BlockQueue<T>::back()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.back();
}
template <typename T>
size_t BlockQueue<T>::size()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size();
}
template <typename T>
size_t BlockQueue<T>::capacity()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return this->capacity_;
}

#endif