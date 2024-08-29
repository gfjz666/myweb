#pragma once
#include <unordered_map>
#include <fcntl.h>    // open
#include <unistd.h>   // close
#include <sys/stat.h> // stat
#include <sys/mman.h> // mmap, munmap
#include "buffer.h"
#include "log.h"

class HTTPresponse
{
public:
    HTTPresponse();
    ~HTTPresponse();
    void init(const std::string &srcDir, std::string &path, bool isKeepAlive = false, int code = -1);
    void makeResponse(buffer &buff);
    void unmapFile_();
    char *file();
    size_t fileLen() const;
    void errorContent(buffer &buff, std::string message);
    int code() const { return code_; }

private:
    void addStateLine_(buffer &buff);
    void addResponseHeader_(buffer &buff);
    void addResponseContent_(buffer &buff);

    void errorHTML_();
    std::string getFileType_();

    int code_;  //HTTP 状态码，默认值为-1
    bool isKeepAlive_; //是否保持连接

    std::string path_;//请求的文件路径
    std::string srcDir_;//资源文件路径

    char *mmFile_;  //内存映射文件的起始地址
    struct stat mmFileStat_;//文件状态信息，用于获取文件大小、修改时间等属性

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;//文件后缀与 MIME 类型的映射
    static const std::unordered_map<int, std::string> CODE_STATUS;//HTTP 状态码与描述信息的映射
    static const std::unordered_map<int, std::string> CODE_PATH;// HTTP 状态码与对应错误页面路径的映射
};
