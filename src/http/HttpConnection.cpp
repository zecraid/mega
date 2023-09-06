#include "HttpConnection.h"
#include <iostream>
const char* HttpConnection::srcDir;
std::atomic<int> HttpConnection::userCount;
bool HttpConnection::isET;

HttpConnection::HttpConnection() {
    readBuff_ = std::make_unique<Buffer>();
    writeBuff_ = std::make_unique<Buffer>();
    writeBuff_->retrieveAll();
    readBuff_->retrieveAll();

    request_ = std::make_unique<HttpRequest>();
    response_ = std::make_unique<HttpResponse>();
    fd_ = -1;
    isClose_ = true;
    addr_ = {0};
};

HttpConnection::~HttpConnection() {
    close();
};

void HttpConnection::init(int sockFd, const sockaddr_in &addr) {
    assert(fd_ > 0);
    userCount++;
    addr_ = addr;
    fd_ = sockFd;
    writeBuff_->retrieveAll();
    readBuff_->retrieveAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
}

void HttpConnection::close() {
    response_->unmapFile();
    if(!isClose_){
        isClose_ = true;
        userCount--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, getIP(), getPort(), (int)userCount);
    }
}

ssize_t HttpConnection::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff_->readFd(fd_, saveErrno);
        if(len <= 0){
            break;
        }
    } while (isET); // ET:边沿触发要一次性全部读出
    return len;
}

// 主要是写入到writeBuff_中
ssize_t HttpConnection::write(int* saveErrno){
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iovCnt_); // 将iov的内容写到fd中
        if (len <= 0) {
            *saveErrno = errno;
            break;
        }
        // 如果是文件，那么就是mmap的文件，需要unmap
        if (iov_[0].iov_len + iov_[1].iov_len == 0) {
            break;
        } else if (static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = (uint8_t*)iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len) {
                writeBuff_->retrieveAll();
                iov_[0].iov_len = 0;
            }
        } else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            writeBuff_->retrieve(len);
        }
    } while ( isET ||toWriteBytes() > 10240); // ET:边沿触发要一次性全部写入
    return len;
}

// 读取数据并解析
bool HttpConnection::process() {
    request_->init();
    if(readBuff_->readableBytes() <= 0) {
        return false;
    } else if(request_->parse(readBuff_.get())) {
        response_->init(srcDir, request_->path(), request_->isKeepAlive(), 200);
    }
    else {
        response_->init(srcDir, request_->path(), false, 400);
    }
    response_->makeResponse(writeBuff_.get());

    // 响应头
    iov_[0].iov_base = const_cast<char*>(writeBuff_->peek());
    iov_[0].iov_len = writeBuff_->readableBytes();
    iovCnt_ = 1;

    // 响应体
    if(response_->fileLen() > 0 && response_->file()) {
        iov_[1].iov_base = response_->file();
        iov_[1].iov_len = response_->fileLen();
        iovCnt_ = 2;
    }

    return true;
}

bool HttpConnection::isKeepAlive() const {
    return request_->isKeepAlive();
}

int HttpConnection::toWriteBytes() {
    return iov_[0].iov_len + iov_[1].iov_len;
}

int HttpConnection::getFd() const {
    return fd_;
}

const char *HttpConnection::getIP() const {
    return inet_ntoa(addr_.sin_addr);
}

int HttpConnection::getPort() const {
    return addr_.sin_port;
}

sockaddr_in HttpConnection::getAddr() const {
    return addr_;
}