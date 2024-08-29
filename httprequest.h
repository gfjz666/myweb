#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex> // 正则表达式
#include <errno.h>
#include <mysql/mysql.h>
#include "buffer.h"
#include "log.h"
#include "sqlconnpool.h"
#include "sqlconnRAII.h"
//解析 HTTP 的请求信息
class HttpRequest
{
public:
    enum PARSE_STATE
    {
        REQUEST_LINE, //行解析
        HEADERS,      //头解析
        BODY,         //体解析
        FINISH,       //完成
    };

    HttpRequest() { Init(); }
    ~HttpRequest() = default;

    void Init();
    bool parse(buffer &buff);

    std::string path() const;
    std::string &path();
    std::string method() const;
    std::string version() const;
    std::string GetPost(const std::string &key) const;
    std::string GetPost(const char *key) const;

    bool IsKeepAlive() const;

private:
    bool ParseRequestLine_(const std::string &line);
    void ParseHeader_(const std::string &line);
    void ParseBody_(const std::string &line);

    void ParsePath_();
    void ParsePost_();
    void ParseFromUrlencoded_();

    static bool UserVerify(const std::string &name, const std::string &pwd, bool isLogin);

    PARSE_STATE state_;
    std::string method_, path_, version_, body_; //分别保存请求的方法、路径、版本和请求体。
    std::unordered_map<std::string, std::string> header_; //存储请求头的键值对
    std::unordered_map<std::string, std::string> post_;   //post请求的数据

    static const std::unordered_set<std::string> DEFAULT_HTML; //存储界面
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;//存储注册登陆
    static int ConverHex(char ch);
};

#endif