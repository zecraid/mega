#include "Acceptor.h"
#include "../log/Log.h"

Acceptor::Acceptor(EventLoop *loop, const char *ip, uint16_t port) {
    socket_ = std::make_unique<Socket>();
    assert(socket_->create() == ST_SUCCESS);
    assert(socket_->setReuseAddr() == ST_SUCCESS);
    assert(socket_->bind(ip,port) == ST_SUCCESS);
    assert(socket_->listen() == ST_SUCCESS);
    socket_->setNonBlocking(); // 默认使用ET模式，那么ListenFd和ClientFd都使用非阻塞
    channel_ = std::make_unique<Channel>(loop, socket_.get());
    // 设置Listen Channel
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    channel_->setReadCallback(cb);
    channel_->enableRead();
    channel_->enableET();
}

void Acceptor::acceptConnection() {
    int client_fd = -1;
    if(socket_->accept(client_fd) != ST_SUCCESS){
        LOG_ERROR("创建ClientFD 失败")
        return;
    }
    LOG_INFO("新连接来咯：fd=%d",client_fd);
    socket_->setNonBlocking();
    if(new_connection_callback_){
        new_connection_callback_(client_fd); // WebServer::newConnection(int fd)
    }
}

void Acceptor::setNewConnectionCallback(const std::function<void(int)> &callback) {
    new_connection_callback_ = std::move(callback);
}