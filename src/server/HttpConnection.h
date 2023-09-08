#ifndef __HTTPCONNECTION_H__
#define __HTTPCONNECTION_H__
#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>
#include "../log/Log.h"
#include "../buffer/Buffer.h"
//#include "../http/HttpRequest.h"
//#include "../http/HttpResponse.h"
#include "../server/EventLoop.h"
#include "../server/State.h"
#include "../server/Socket.h"
#include "../server/Channel.h"

class EventLoop;
class Socket;
class Channel;
class Buffer;
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
    void process();

    int getFd() const;
    std::string getAddr() const; // 获取完整Client IP地址
    void setRecvCallback();

private:
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    State state_;
    std::unique_ptr<Buffer> read_buf_;
    std::unique_ptr<Buffer> write_buf_;

};


#endif //!__HTTPCONNECTION_H__
