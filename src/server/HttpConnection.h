#ifndef __HTTPCONNECTION_H__
#define __HTTPCONNECTION_H__
#include <functional>
#include <memory>
#include "State.h"
#include "EventLoop.h"
#include "../buffer/Buffer.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "Socket.h"
#include "Channel.h"
class Buffer;
class Socket;
class EventLoop;
class Channel;
class HttpResponse;
class HttpRequest;
class HttpConnection {
public:
    enum ConnState{
        INVALID = 0,
        CONNECTING,
        CONNECTED,
        CLOSED,
    };
    HttpConnection(int fd, EventLoop *loop);
    ~HttpConnection();

    ssize_t read(int *saveErrno);
    ssize_t write(int *saveErrno);

    void setCloseConnectionCallback(std::function<void(int)> const &fn);
    void closeConnection();

    int getFd() const;
    void readRequest();
    bool processRequest();
    void sendResponse();

    int writeBytesLength();
    bool isKeepAlive();

private:
    void dealRead_();
    void dealWrite_();

public:
    static const char* srcDir;

private:
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    int iovCnt_;
    struct iovec iov_[2];

    ConnState state_;
    std::unique_ptr<Buffer> read_buf_;
    std::unique_ptr<Buffer> write_buf_;

    std::unique_ptr<HttpRequest> request_;
    std::unique_ptr<HttpResponse> response_;

    std::function<void(int)> close_connection_;

};


#endif //~__HTTPCONNECTION_H__
