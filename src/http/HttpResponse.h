#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap

#include "../buffer/Buffer.h"
#include "../log/Log.h"

class Buffer;
class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    void makeResponse(Buffer *buff);
    void unmapFile();
    char* file();
    size_t fileLen() const;
    void errorContent(Buffer *buff, std::string message);
    int code() const { return code_; }

private:
    void addStateLine_(Buffer *buff);
    void addHeader_(Buffer *buff);
    void addContent_(Buffer *buff);

    void errorHtml_();
    std::string getFileType_();

private:
    int code_;
    bool isKeepAlive_;

    std::string path_;
    std::string srcDir_;

    char* mmFile_;
    struct stat mmFileStat_;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;  // 后缀类型集
    static const std::unordered_map<int, std::string> CODE_STATUS;          // 编码状态集
    static const std::unordered_map<int, std::string> CODE_PATH;            // 编码路径集
};


#endif //__HTTP_RESPONSE_H__