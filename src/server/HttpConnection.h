#ifndef __HTTPCONNECTION_H__
#define __HTTPCONNECTION_H__
#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>
#include "../log/Log.h"
#include "../buffer/Buffer.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "EventLoop.h"
#include "State.h"
#include "Socket.h"
#include "Channel.h"

class EventLoop;
class Socket;
class Channel;
class Buffer;
class HttpRequest;
class HttpResponse;
class HttpConnection {
public:
    enum State {
        INVALID = 0,
        CONNECTING,
        CONNECTED,
        CLOSED,
    };
    HttpConnection(int fd, EventLoop *loop);
    ~HttpConnection();
    void close();

    void init(int fd, EventLoop *loop);
    ssize_t read(int *saveErrno);
    ssize_t write(int *saveErrno);
    void closeConnection();
    void setCloseConnectionCallback(std::function<void(int)> const &fn);
    void setRequestRecvCallback();
//    void setResponseSendCallback();

    bool process();

    int getFd() const;
    std::string getAddr() const; // 获取完整Client IP地址

    int writeBytesLength();
    bool isKeepAlive() const;

public:
    static bool isET;
    static const char* srcDir;
//    static std::atomic<int> userCount;  // 原子，支持锁

private:
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    int iovCnt_;
    struct iovec iov_[2];

    bool isClose_;

    State state_;
    std::unique_ptr<Buffer> read_buf_;
    std::unique_ptr<Buffer> write_buf_;

    std::unique_ptr<HttpRequest> request_;
    std::unique_ptr<HttpResponse> response_;

    std::function<void(int)> close_connection_;
};


#endif //!__HTTPCONNECTION_H__
