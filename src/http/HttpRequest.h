#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>    // 正则表达式
#include <errno.h>
#include <mysql/mysql.h>  //mysql

#include "../buffer/Buffer.h"
#include "../logger/Logger.h"
#include "../pool/SqlConnectionPool.h"
#include "../pool/SqlConnectionRAII.h"

class HttpBuffer;
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

    void init();
    bool parse(Buffer* buff);

    std::string path() const;
    std::string& path();
    std::string method() const;
    std::string version() const;
    std::string getPost(const std::string& key) const;
    std::string getPost(const char* key) const;

    bool isKeepAlive() const;

private:
    bool parseRequestLine_(const std::string& line);  // 处理请求行
    void parseHeader_(const std::string& line);       // 处理请求头
    void parseBody_(const std::string& line);         // 处理请求体

    void parsePath_();                                // 处理请求路径
    void parsePost_();                                // 处理Post事件
    void parseFromUrlencoded_();                      // 从url种解析编码

    static bool userLogin(const std::string& name, const std::string& pwd); // 用户登录
    static bool userRegister(const std::string& name, const std::string& pwd); // 用户注册

    PARSE_STATE state_;
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int converHex(char ch); // 16进制转换为10进制
};


#endif //__HTTP_REQUEST_H__