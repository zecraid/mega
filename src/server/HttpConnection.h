#ifndef __HTTPCONNECTION_H__
#define __HTTPCONNECTION_H__

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>
#include <cassert>
#include <memory>
#include "Socket.h"
#include "../log/Log.h"
#include "../buffer/Buffer.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
class Buffer;
class HttpRequest;
class HttpResponse;
class Socket;
class HttpConnection {
public:
    HttpConnection();
    ~HttpConnection();

//    void init(int sockFd, const sockaddr_in& addr);
    void init(int sockFd);
    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);
    void close();
    int getFd() const;
    std::string getIP() const;
    bool process();
    int writeBytesLength();
    bool isKeepAlive() const;


public:
    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;  // 原子，支持锁

private:
    std::unique_ptr<Socket> socket_;

    bool isClose_;

    int iovCnt_;
    struct iovec iov_[2];

    std::unique_ptr<Buffer> read_buff_;
    std::unique_ptr<Buffer> write_buff_;

    std::unique_ptr<HttpRequest> request_;
    std::unique_ptr<HttpResponse> response_;
};

#endif //!__HTTPCONNECTION_H__
