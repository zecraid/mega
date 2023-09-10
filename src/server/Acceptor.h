#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__
#include <functional>
#include <memory>
#include "State.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
class EventLoop;
class Socket;
class Channel;
class Acceptor
{
public:
    Acceptor(EventLoop *loop, const char* ip, uint16_t port);
    ~Acceptor() = default;
    void acceptConnection();
    void setNewConnectionCallback(std::function<void(int)> const &callback);

private:
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    std::function<void(int)> new_connection_callback_;

};

#endif //__ACCEPTOR_H__