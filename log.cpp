#include "log.h"
using namespace std;

Log::Log()
{
    // 初始化日志类的成员对象
    fp_ = nullptr;
    deque_ = nullptr;
    writeThread_ = nullptr;
    lineCount_ = 0;
    toDay_ = 0;
    isAsync_ = false;
}
void Log::AppendLogLevelTitle_(int level)
{
    switch (level)
    {
    case 0:
        buff_.append("[debug]: ", 9);
        break;
    case 1:
        buff_.append("[info] : ", 9);
        break;
    case 2:
        buff_.append("[warn] : ", 9);
        break;
    case 3:
        buff_.append("[error]: ", 9);
        break;
    default:
        buff_.append("[info] : ", 9);
        break;
    }
}
Log::~Log()
{
    // 清空阻塞队列，关闭文件
    if (writeThread_ && writeThread_->joinable()) // joinable，检查线程是否可加入
    {
        while (!deque_->empty())
        {
            deque_->flush(); // 唤醒消费者，处理掉剩下的任务
        };
        deque_->close();
        writeThread_->join();
    }
    if (fp_)
    {
        lock_guard<mutex> locker(mtx_);
        flush();
        fclose(fp_);
    }
}
void Log::AsyncWrite_()
{
    // 异步写入日志数据到文件中
    string str = "";
    while (deque_->pop(str))
    {
        lock_guard<mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}
void Log::write(int level, const char *format, ...)
{
    /*要用于将日志写入缓冲区或文件：
     获取当前时间，并根据日期和日志行数决定是否创建新日志文件。
     根据日志级别添加相应的日志标题。
     日志内容格式化写入缓冲区，如果是异步模式，将日志内容加入阻塞队列，否则直接写入文件。
    */
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr); // linux下的计时方法
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;

    if (toDay_ != t.tm_mday || (lineCount_ && (lineCount_ % MAX_LINES == 0)))
    {
        // 如果当前日期 toDay_ 与系统日期不一致，或者日志文件的行数超过了MAX_LINES，则需要创建新的日志文件
        unique_lock<mutex> locker(mtx_);
        locker.unlock();

        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        // 年_月_日 日期写入tail
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (toDay_ != t.tm_mday)
        {
            // 时间不匹配，则替换为最新的日志文件名
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        }
        else
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (lineCount_ / MAX_LINES), suffix_);

        locker.lock();
        flush();
        fclose(fp_);
        fp_ = fopen(newFile, "a");
        assert(fp_ != nullptr);
    }
    // 日志文件先写入buffer
    {
        unique_lock<mutex> locker(mtx_);
        lineCount_++;
        int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                         t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                         t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);

        buff_.HasWritten(n);
        AppendLogLevelTitle_(level);
        va_start(vaList, format);
        int m = vsnprintf(buff_.BeginWrite(), buff_.WritableBytes(), format, vaList);
        va_end(vaList);

        buff_.HasWritten(m);
        buff_.append("\n\0", 2);

        if (isAsync_ && deque_ && !deque_->full())
        {
            // 异步则放进队列里面
            deque_->push_back(buff_.RetrieveAllTostr());
        }
        else
        {
            // 同步则直接存进文件
            fputs(buff_.Peek(), fp_);
        }
        buff_.RetrieveAll(); // 清空buff
    }
}
void Log::flush()
{
    // 刷新日志缓冲区内容,阻塞队列写入内容
    if (isAsync_)
    {
        deque_->flush();
    }
    fflush(fp_);
}
int Log::GetLevel()
{
    lock_guard<mutex> locker(mtx_);
    return level_;
}
void Log::SetLevel(int level)
{
    lock_guard<mutex> locker(mtx_);
    level_ = level;
}
void Log::init(int level, const char *path, const char *suffix, int maxQueueCapacity)
{
    isOpen_ = true;
    level_ = level;
    path_ = path;
    suffix_ = suffix;
    if (maxQueueCapacity > 0)
    {
        // 启用异步日志
        isAsync_ = true;
        if (!deque_) // 如果阻塞队列还未创建，则创建一个新的队列
        {
            unique_ptr<BlockQueue<string>> newQueue(new BlockQueue<string>);
            // unique_ptr<BlockQueue<std::string>> newQueue(new BlockQueue<std::string>(maxQueueCapacity));
            //  独占指针只能转移构造
            deque_ = move(newQueue);
            unique_ptr<std::thread> newThread(new thread(FlushLogThread));
            writeThread_ = move(newThread);
        }
    }
    else
        isAsync_ = false;

    lineCount_ = 0;
    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;

    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s",
             path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);

    toDay_ = t.tm_mday;
    {
        lock_guard<mutex> locker(mtx_);
        buff_.RetrieveAll(); // 清空buffer
        if (fp_)             // 重新打开
        {
            flush();
            fclose(fp_);
        }
        fp_ = fopen(fileName, "a");
        if (fp_ == nullptr)
        {
            mkdir(fileName, 0777);
            fp_ = fopen(fileName, "a"); // 生成目录文件（最大权限）
        }
        assert(fp_ != nullptr);
    }
}
Log *Log::Instance()
{
    // 懒汉模式,无需加锁
    static Log inst;
    return &inst;
}

void Log::FlushLogThread()
{
    // 启动一个独立的线程来异步写日志
    Log::Instance()->AsyncWrite_();
}
