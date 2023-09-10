#include "Acceptor.h"
#include "../log/Log.h"

Acceptor::Acceptor(EventLoop *loop, const char *ip, uint16_t port) {
    socket_ = std::make_unique<Socket>();
    assert(socket_->create() == RC_SUCCESS);
    assert(socket_->setReuseAddr() == RC_SUCCESS);
    assert(socket_->bind(ip,port) == RC_SUCCESS);
    assert(socket_->listen() == RC_SUCCESS);
    socket_->setNonBlocking(); // 默认使用ET模式，那么ListenFd和ClientFd都使用非阻塞
    channel_ = std::make_unique<Channel>(loop, socket_.get());
    // 设置Listen Channel
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    channel_->setReadCallback(cb);
    channel_->enableRead();
    channel_->enableET();
}

ST Acceptor::acceptConnection() {
    int client_fd = -1;
    if(socket_->accept(client_fd) != RC_SUCCESS){
        return RC_ACCEPTOR_ERROR;
    }
    if(new_connection_callback_){
        new_connection_callback_(client_fd); // WebServer::newConnection(int fd)
    }
    return ST_SUCCESS;
}

void Acceptor::setNewConnectionCallback(const std::function<void(int)> &callback) {
    new_connection_callback_ = std::move(callback);
}
