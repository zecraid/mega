#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__
#include <functional>
#include <memory>
#include "State.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
class Acceptor {
public:
    Acceptor(EventLoop *loop, const char* ip, uint16_t port);
    ~Acceptor() = default;
    ST acceptConnection(); // ListenChannel的读回调函数：接受新连接
    void setNewConnectionCallback(std::function<void(int)> const &callback); // 自定义新连接的回调函数，并在acceptConnection中执行

private:
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    std::function<void(int)> new_conn_callback_;
};


#endif //!__ACCEPTOR_H__
