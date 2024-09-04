#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <assert.h>
#include <chrono>
#include "log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode
{
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    bool operator<(const TimerNode &t)
    {
        return expires < t.expires;
    }
};

class HeapTimer
{
private:
    std::vector<TimerNode> heap_;
    std::unordered_map<int, size_t> ref_;   // 映射fd对应的位置
    void del_(size_t index);                // 删除索引为 i 的任务节点，并调整堆以保持堆性质
    void siftup_(size_t i);                 // 向上操作，将索引为 i 的节点向上调整，以维护堆的性质
    bool siftdown_(size_t index, size_t n); // 向下操作，将索引为 index 的节点向下调整，以维护堆的性质
    void swapNode_(size_t i, size_t j);     // 交换堆中索引为 i 和 j 的两个节点的位置

public:
    HeapTimer() { heap_.reserve(64); }
    ~HeapTimer() { clear(); }

    void adjust(int id, int newExpires);                      // 调整id的到期时间
    void add(int id, int timeOut, const TimeoutCallBack &cb); // 添加一个定时器
    void doWork(int id);                                      // 执行给定 id 的定时任务的回调函数并删除该任务
    void clear();                                             // 清除所有定时认为
    void tick();                                              // 检查堆中所有任务的到期时间，并执行所有已到期的任务
    void pop();                                               // 删除堆顶的任务
    int GetNextTick();                                        // 获取下一个任务的剩余时间，以毫秒为单位
};

#endif // HEAP_TIMER_H