//#include "Connection.h"
//#include "Socket.h"
//#include "Channel.h"
//#include "../buffer/Buffer.h"
//#include "../log/Log.h"
//#include "Util.h"
//#include <unistd.h>
//#include <string.h>
//#include <stdio.h>
//#include <errno.h>
//#include <memory>
//#include <cstring>
//
//#define READ_BUFFER 1024
//
//// 当有一个client连接时，就会创建一个Connection（只有clientFd有）
//// 每一个Connection包含一个Socket和Channel，以及读写buffer
//Connection::Connection(int fd, EventLoop *loop) {
//    socket_ = std::make_unique<Socket>();
//    socket_->setFd(fd);
//    if(loop != nullptr){
//        channel_ = std::make_unique<Channel>(loop, socket_.get());
//        channel_->enableRead();
//        channel_->enableET();
//    }
//    read_buf_ = std::make_unique<Buffer>();
//    send_buf_ = std::make_unique<Buffer>();
//    state_ = State::CONNECTED;
//}
//
//ST Connection::read() {
//    if(state_ != State::CONNECTED){
//        LOG_WARN("Connection is not connected, can not read");
//        return ST_CONNECTION_ERROR;
//    }
//    assert(state_ == State::CONNECTED);
//    read_buf_->retrieveAll();
//    if(socket_->isNonBlocking()){
//        return readNonBlocking_();
//    } else {
//        return readBlocking_();
//    }
//}
//
//ST Connection::write() {
//    if(state_ != State::CONNECTED){
//        LOG_WARN("Connection is not connected, can not write");
//        return ST_CONNECTION_ERROR;
//    }
//    assert(state_ == State::CONNECTED);
//    ST st = ST_UNDEFINED;
//    if(socket_->isNonBlocking()){
//        st = writeNonBlocking_();
//    } else {
//        st = writeBlocking_();
//    }
//    send_buf_->retrieveAll();
//    return st;
//}
//
//ST Connection::readNonBlocking_() {
//    int sockfd = socket_->getFd();
//    char buf[1024]; // 这个buf大小无所谓
//    while(true){ // 由于使用非阻塞读，需要判断EAGAIN、EWOULDBLOCK...
//        memset(buf, 0, sizeof(buf));
//        ssize_t bytes_read = ::read(sockfd, buf, sizeof(buf));
//        if(bytes_read > 0){
//            read_buf_->append(buf, bytes_read);
//        } else if(bytes_read == -1 && errno == EINTR){ // 程序正常中断、继续读取
//            LOG_INFO("continue reading");
//            continue;
//        } else if(bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  // 非阻塞IO，这个条件表示数据全部读取完毕
//            break;
//        } else if(bytes_read == 0){ // EOF，客户端断开连接
//            LOG_INFO("read EOF, client fd %d disconnected", sockfd);
//            state_ = State::CLOSED;
//            close();
//            break;
//        } else {
//            LOG_INFO("Other error on client fd %d", sockfd);
//            state_ = State::CLOSED;
//            close();
//            break;
//        }
//    }
//    return ST_SUCCESS;
//}
//
//ST Connection::writeNonBlocking_() {
//    int sockfd = socket_->getFd();
//    char buf[send_buf_->readableBytes()];
//    memcpy(buf, send_buf_->peek(), send_buf_->readableBytes());
//    int data_size = send_buf_->readableBytes();
//    int date_left = data_size;
//    while(date_left > 0){
//        ssize_t bytes_write = ::write(sockfd, buf + data_size - date_left, date_left);
//        if(bytes_write == -1 && errno == EINTR){
//            LOG_INFO("continue writing");
//            continue;
//        }
//        if(bytes_write == -1 && errno == EAGAIN){
//            break;
//        }
//        if(bytes_write == -1){
//            LOG_INFO("Other error on client fd %d", sockfd);
//            state_ = State::CLOSED;
//            break;
//        }
//        date_left -= bytes_write;
//    }
//    return ST_SUCCESS;
//}
//
//ST Connection::readBlocking_() {
//    int sockfd = socket_->getFd();
//    char buf[1024];
//    if()
//}