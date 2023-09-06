#ifndef __HTTP_CONNECTION_H__
#define __HTTP_CONNECTION_H__

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>

#include "../log/Log.h"
#include "../buffer/Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

/*
进行读写数据并调用httprequest 来解析数据以及httpresponse来生成响应
*/
class HttpConnection {
public:
    HttpConnection();
    ~HttpConnection();

    void init(int sockFd, const sockaddr_in& addr);
    ssize_t write(int* saveErrno);
    ssize_t read(int* saveErrno);
    void close();
    bool process();

    int toWriteBytes();
    bool isKeepAlive() const;

    int getFd() const;
    int getPort() const;
    const char* getIP() const;
    sockaddr_in getAddr() const;

    static const char* srcDir;
    static std::atomic<int> userCount;  // 原子，支持锁

private:
    int fd_;
    struct sockaddr_in addr_;

    bool isClose_;

    int iovCnt_;
    struct iovec iov_[2];

    std::unique_ptr<Buffer> readBuff_; //读缓冲区
    std::unique_ptr<Buffer> writeBuff_; //写缓冲区

    std::unique_ptr<HttpRequest> request_;
    std::unique_ptr<HttpResponse> response_;

};

#endif //__HTTP_CONNECTION_H__