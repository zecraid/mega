#include "HttpConnection.h"

HttpConnection::HttpConnection(int fd, EventLoop *loop) {
    socket_ = std::make_unique<Socket>();
    socket_->setFd(fd);
    if(loop != nullptr){
        channel_ = std::make_unique<Channel>( loop,socket_.get());
        channel_->enableRead();
        channel_->enableET();
    }
    read_buf_ = std::make_unique<Buffer>();
    write_buf_ = std::make_unique<Buffer>();
    read_buf_->retrieveAll();
    write_buf_->retrieveAll();
    state_ = State::CONNECTED;
}

HttpConnection::~HttpConnection() {
    close();
}

void HttpConnection::close() {
    ::close(socket_->getFd());
}

void HttpConnection::init(int fd, EventLoop *loop) {
    assert(fd > 0);
    socket_->setFd(fd);
    read_buf_->retrieveAll();
    write_buf_->retrieveAll();
    LOG_INFO("HttpConnection init");
}

ssize_t HttpConnection::read(int *saveErrno) {
    ssize_t len = -1;
    do {
        len = read_buf_->readFd(getFd(), saveErrno);
        if(len <= 0){
            *saveErrno = errno;
            break;
        }
    } while (true);
    return len;
}

ssize_t HttpConnection::write(int *saveErrno) {
    ssize_t len = -1;
    do {
        len = write_buf_->writeFd(getFd(), saveErrno);
        if(len <= 0){
            *saveErrno = errno;
            break;
        }
    } while (true);
    return len;
}

void HttpConnection::process() {
    std::string content = read_buf_->retrieveAllToStr();
    LOG_INFO("msg get:%s",content.c_str());
    write_buf_->append(content);
    write_buf_->append("+++hello");
}

void HttpConnection::setRecvCallback() {
    std::function<void()> cb = [this](){
        int saveErrno = -1;
        read(&saveErrno);
        process();
        write(&saveErrno);
    };
    channel_->setReadCallback(cb);
}

int HttpConnection::getFd() const {
    return socket_->getFd();
}

std::string HttpConnection::getAddr() const {
    return socket_->getAddr();
}



