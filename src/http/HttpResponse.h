#ifndef __HTTPRESPONSE_H__
#define __HTTPRESPONSE_H__

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "../buffer/Buffer.h"
#include "../log/Log.h"
class Buffer;
class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1); // 初始化Response
    void makeResponse(Buffer *buff); // 构造Response
    void unmapFile(); // 清空文件的内存空间映射
    char* file(); // 返回文件指针
    size_t fileLen() const; // 文件长度
    void errorContent(Buffer *buff, std::string message); // 构造错误页面的Conetent
    int code() const { return code_; } // 返回Response码

private:
    void addStateLine_(Buffer *buff); // 添加 HTTP Response的状态行
    void addHeader_(Buffer *buff); // 添加Resqponse头
    void addContent_(Buffer *buff); // 添加Response内容

    void errorHtml_(); //
    std::string getFileType_(); // 获取文件类型

private:
    int code_;
    bool isKeepAlive_;

    std::string path_; // 请求路径
    std::string srcDir_; // 资源路径（和path_一起拼接成完整的文件路径）

    char* mmFile_; // 文件指针
    struct stat mmFileStat_; // 文件状态，包括文件大小等信息

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;  // 后缀类型集
    static const std::unordered_map<int, std::string> CODE_STATUS;          // 编码状态集
    static const std::unordered_map<int, std::string> CODE_PATH;            // 编码路径集
};


#endif //!__HTTPRESPONSE_H__