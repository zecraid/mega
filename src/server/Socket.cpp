#include "Socket.h"
#include "Util.h"
#include "../log/Log.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <cassert>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
Socket::Socket() : fd_(-1){};

Socket::Socket(int fd) : fd_(fd){
    Util::errif(fd == -1, "Socket create error");
}

Socket::~Socket() {
    if(fd_ != -1){
        close(fd_);
        fd_ = -1;
    }
}

int Socket::getFd() const {
    return fd_;
}

void Socket::setFd(int fd) {
    fd_ = fd;
}

// return IP:PORT
std::string Socket::getAddr() {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t len = sizeof(addr);
    if(getpeername(fd_, (struct sockaddr *)&addr, &len) == -1){
        return "";
    }
    std::string ret(inet_ntoa(addr.sin_addr));
    ret += ":";
    ret += std::to_string(htons(addr.sin_port));
    return ret;
}

ST Socket::create() {
    assert(fd_ == -1);
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ == -1) {
        LOG_ERROR("Create socket fail");
        return ST_SOCKET_ERROR;
    }
    return ST_SUCCESS;
}

ST Socket::bind(const char *ip, uint16_t port) const {
    assert(fd_ != -1);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    if (::bind(fd_, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        LOG_ERROR("Bind socket error");
        return ST_SOCKET_ERROR;
    }
    return ST_SUCCESS;
}

ST Socket::listen() const {
    assert(fd_ != -1);
    if (::listen(fd_, SOMAXCONN) == -1) {
        LOG_ERROR("Listen socket error");
        return ST_SOCKET_ERROR;
    }
    return ST_SUCCESS;
}

ST Socket::accept(int &client_fd) {
    assert(fd_ != -1);
    client_fd = ::accept(fd_, nullptr, nullptr);
    if (client_fd == -1) {
        LOG_ERROR("Accept socket error");
        return ST_SOCKET_ERROR;
    }
    return ST_SUCCESS;
}

ST Socket::connect(const char *ip, uint16_t port) const {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    if (::connect(fd_, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        LOG_ERROR("Connect socket error");
        return ST_SOCKET_ERROR;
    }
    return ST_SUCCESS;
}

ST Socket::setNonBlocking() {
    if (fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL) | O_NONBLOCK) == -1) {
        LOG_ERROR("Socket set non-blocking failed");
        return ST_SOCKET_ERROR;
    }
    return ST_SUCCESS;
}

bool Socket::isNonBlocking() const {
    return (fcntl(fd_, F_GETFL) & O_NONBLOCK) != 0;
}

ST Socket::setReuseAddr() {
    int opt = 1;
    if(setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
        LOG_ERROR("set ReuseAddr error");
        return ST_SOCKET_ERROR;
    }
    return ST_SUCCESS;
}