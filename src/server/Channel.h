#ifndef __CHANNEL_H__
#define __CHANNEL_H__
#include <sys/epoll.h>
#include <functional>
#include "EventLoop.h"
#include "Socket.h"
class Socket;
class EventLoop;
class Channel
{

public:
    Channel(EventLoop *_loop, Socket *socket);
    ~Channel();

    void handleEvent();
    void enableRead();
    void enableWrite();
    void enableET();

    int getFd();
    Socket* getSocket();
    int getListenEvents();
    int getReadyEvents();
    bool getExist();
    void setExist(bool in = true);

    void setReadyEvents(int event);
    void setReadCallback(std::function<void()> const &callback);
    void setWriteCallback(std::function<void()> const &callback);

    static const int READ_EVENT;
    static const int WRITE_EVENT;
    static const int ET;

private:
    EventLoop *loop_;
    Socket *socket_;
    int listen_events_{0};
    int ready_events_{0};
    bool exist_{false};
    std::function<void()> read_callback_;
    std::function<void()> write_callback_;

};

#endif