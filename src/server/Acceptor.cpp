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

ST Acceptor::acceptConnection() {
    int client_fd = -1;
    if(socket_->accept(client_fd) != ST_SUCCESS){
        return ST_ACCEPTOR_ERROR;
    }
    LOG_INFO("新连接来咯：fd=%d",client_fd);
    if(new_connection_callback_){
        new_connection_callback_(client_fd); // WebServer::newConnection(int fd)
    }
    return ST_SUCCESS;
}

void Acceptor::setNewConnectionCallback(const std::function<void(int)> &callback) {
    new_connection_callback_ = std::move(callback);
}
