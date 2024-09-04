#ifndef LOG_H
#define LOG_H
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>     // vastart va_end
#include <assert.h>
#include <sys/stat.h>   // mkdir
#include <mutex>
#include "buffer.h"
#include "blockqueue.h"
class Log
{
public:
    // 初始化日志实例（阻塞队列最大容量、日志保存路径、日志文件后缀）
    void init(int level, const char *path = "./bin", const char *suffix = ".log", int maxQueueCapacity = 1024);
    static Log *Instance();       // 单例模式，为该类的唯一对象
    static void FlushLogThread(); // 异步写日志公有方法

    void write(int level, const char *format, ...);
    void flush();                     // 刷新日志缓冲区，将缓冲区内容写入文件。
    int GetLevel();                   // 获取当前日志等级
    void SetLevel(int level);         // 设置日志等级
    bool IsOpen() { return isOpen_; } // 判断日志是否打开

private:
    Log();
    void AppendLogLevelTitle_(int level); // 根据日志等级附加相应的日志标题。
    virtual ~Log();
    void AsyncWrite_();                   // 异步写日志方法,将日志内容从阻塞队列写入日志文件
private:
    static const int LOG_PATH_LEN = 256; // 日志文件最长文件名
    static const int LOG_NAME_LEN = 256; // 日志最长名字
    static const int MAX_LINES = 50000;  // 日志文件内的最长日志条数
    const char *path_;                   // 路径
    const char *suffix_;                 // 后缀
    int MAX_LINES_;                      // 最大日志行数
    int lineCount_;                      // 日志行数记录
    int toDay_;                          // 当前日期
    bool isOpen_;                        // 判断是否打开
    buffer buff_;                        // 定义缓冲区
    int level_;                          // 日志等级
    bool isAsync_;                       // 是否开启同步人日志
    FILE *fp_;
    std::unique_ptr<BlockQueue<std::string>> deque_; // 异步阻塞队列
    std::unique_ptr<std::thread> writeThread_;       // 写线程的智能指针
    std::mutex mtx_;                                 // 同步日志的互斥量
};
// 基础宏，用于根据日志级别和内容输出日志
#define LOG_BASE(level, format, ...)                   \
    do                                                 \
    {                                                  \
        Log *log = Log::Instance();                    \
        if (log->IsOpen() && log->GetLevel() <= level) \
        {                                              \
            log->write(level, format, ##__VA_ARGS__);  \
            log->flush();                              \
        }                                              \
    } while (0);
//日志等级宏
#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);
#endif