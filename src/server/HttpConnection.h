#ifndef __HTTPCONNECTION_H__
#define __HTTPCONNECTION_H__

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>
#include <cassert>
#include <memory>
#include "../log/Log.h"
#include "../buffer/Buffer.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
class Buffer;
class HttpRequest;
class HttpResponse;
class HttpConnection {
public:
    HttpConnection();
    ~HttpConnection();

    void init(int sockFd); // 通过Fd创建HttpConnection
    ssize_t read(int* saveErrno); // 从fd读取数据到read_buff_
    ssize_t write(int* saveErrno); // 将write_buff_数据写入fd
    void close(); // 关闭一个HttpConnection
    int getFd() const; // 获取HttpConnection的fd
    std::string getAddress() const; // 获取完整的Client的地址（IP:Port）
    bool process(); // 解析Request请求，组装Response
    int writeBytesLength(); // 写入数据的总长度
    bool isKeepAlive() const; // 返回是否位Keep-alive状态

public:
    static bool isET; // ET模式
    static const char* srcDir; // 资源路径
    static std::atomic<int> userCount;  // 原子，支持锁

private:
    int fd_;

    bool isClose_;

    int iovCnt_;
    struct iovec iov_[2];

    std::unique_ptr<Buffer> read_buff_;
    std::unique_ptr<Buffer> write_buff_;

    std::unique_ptr<HttpRequest> request_;
    std::unique_ptr<HttpResponse> response_;
};

#endif //!__HTTPCONNECTION_H__
