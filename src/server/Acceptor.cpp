#include "Acceptor.h"
#include "Socket.h"
#include "Channel.h"
#include "../log/Log.h"
#include <stdio.h>
#include <cstring>
#include "State.h"

Acceptor::Acceptor(EventLoop *loop, const char* ip, uint16_t port) {
    socket_ = std::make_unique<Socket>();
    assert(socket_->create() == ST_SUCCESS);
    assert(socket_->setReuseAddr() == ST_SUCCESS);
    assert(socket_->bind(ip,port) == ST_SUCCESS);
    assert(socket_->listen() == ST_SUCCESS);
    
    channel_ = std::make_unique<Channel>(loop, socket_.get());

    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    channel_->setReadCallback(cb);
    channel_->enableRead();
}

ST Acceptor::acceptConnection() {
    int client_fd = -1;
    if(socket_->accept(client_fd) != ST_SUCCESS){
        return ST_ACCEPTOR_ERROR;
    }
    LOG_INFO("新连接来咯：fd=%d",client_fd);
    socket_->setNonBlocking(); // 新接受到的连接设置为非阻塞式
    if(new_connection_callback_){
        new_connection_callback_(client_fd); // WebServer::newConnection(int fd)
    }
    return ST_SUCCESS;
}


void Acceptor::setNewConnectionCallback(std::function<void(int)> const &callback){
    new_connection_callback_ = std::move(callback);
}