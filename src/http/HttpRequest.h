#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>    // 正则表达式
#include <errno.h>
#include <mysql/mysql.h>  //mysql

#include "../buffer/Buffer.h"
#include "../log/Log.h"
#include "../pool/SqlConnectionPool.h"
#include "../pool/SqlConnectionRAII.h"

class Buffer;
class HttpRequest {
public:
    enum PARSE_STATE{
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };
    HttpRequest() { init(); }
    ~HttpRequest() = default;

    void init(); // 初始化Request
    bool parse(Buffer* buff); // 有限状态机解析HTPP请求

    std::string path() const; // 请求路径
    std::string& path();
    std::string method() const; // HTTP请求方式
    std::string version() const; // HTTP版本号
    std::string getPost(const std::string& key) const; // 传入key，获取Post请求的数据value
    std::string getPost(const char* key) const;

    bool isKeepAlive() const; // Request 是否为keep-alive

private:
    bool parseRequestLine_(const std::string& line);  // 处理请求行
    void parseHeader_(const std::string& line);       // 处理请求头
    void parseBody_(const std::string& line);         // 处理请求体

    void parsePath_();                                // 处理请求路径
    void parsePost_();                                // 处理Post事件
    void parseFromUrlencoded_();                      // 从url种解析编码

    static bool userLogin(const std::string& name, const std::string& pwd); // 用户登录
    static bool userRegister(const std::string& name, const std::string& pwd); // 用户注册

private:
    PARSE_STATE state_;
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int converHex(char ch); // 16进制转换为10进制
};


#endif //!__HTTPREQUEST_H__