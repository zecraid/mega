#include "Acceptor.h"
#include "../log/Log.h"
#include <cstring>

Acceptor::Acceptor(EventLoop *loop, const char *ip, uint16_t port) {
    socket_ = std::make_unique<Socket>();
    assert(socket_->create() == ST_SUCCESS); // 创建ListenFd
    assert(socket_->setReuseAddr() == ST_SUCCESS); // 设置端口复用
    assert(socket_->bind(ip, port) == ST_SUCCESS); // 绑定地址结构
    assert(socket_->listen() == ST_SUCCESS); // 绑定MAXCONN
//    socket_->setNonBlocking();

    channel_ = std::make_unique<Channel>(loop, socket_.get()); // 为ListenFd创建channel
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this); // 由于是ListenFd，它的READ事件就是创建Client
    channel_->setReadCallback(cb); // 设定写回调函数（创建Client）
    channel_->enableRead(); // 将READ事件绑定上，然后加入红黑树
}

// ListenChannel的读回调函数：接受新连接
ST Acceptor::acceptConnection() {
    int client_fd = -1;
    if(socket_->accept(client_fd) != ST_SUCCESS){
        return ST_ACCEPTOR_ERROR;
    }
    // 非阻塞 yes or no
    if(new_conn_callback_){
        new_conn_callback_(client_fd);
    }
    return ST_SUCCESS;
}

void Acceptor::setNewConnectionCallback(const std::function<void(int)> &callback) {
    new_conn_callback_ = std::move(callback);
}
