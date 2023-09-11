#include "HttpConnection.h"
const char* HttpConn::srcDir;
std::atomic<int> HttpConn::userCount;
bool HttpConn::isET;

HttpConnection::HttpConnection() {
    fd_ = -1;
    addr_ = { 0 };
    isClose_ = true;
}

HttpConnection::HttpConnection() {
    close();
}

void HttpConnection::init(int sockFd, const sockaddr_in &addr) {
    assert(fd > 0);
    userCount++;
    addr_ = addr;
    fd_ = fd;
    writeBuff_.retrieveAll();
    readBuff_.retrieveAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
}

void HttpConnection::close() {
    response_.UnmapFile();
    if(isClose_ == false){
        isClose_ = true;
        userCount--;
        ::close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
    }
}

int HttpConnection::getFd() const {
    return fd_;
}

sockaddr_in HttpConnection::getAddr() const {
    return addr_;
}

int HttpConnection::getPort() const {
    return addr_.sin_port;
}

ssize_t HttpConnection::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff_.readFd(fd_, saveErrno);
        if (len <= 0) {
            break;
        }
    } while (isET); // ET:边沿触发要一次性全部读出
    return len;
}

// 主要采用writev连续写函数
ssize_t HttpConnection::write(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iovCnt_);   // 将iov的内容写到fd中
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        if(iov_[0].iov_len + iov_[1].iov_len  == 0) { break; } /* 传输结束 */
        else if(static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len) {
                writeBuff_.retrieveAll();
                iov_[0].iov_len = 0;
            }
        }
        else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            writeBuff_.moveReadPos(len);
        }
    } while(isET || writeBytesLength() > 10240);
    return len;
}

bool HttpConnection::process() {
    request_.init();
    if(readBuff_.readableBytes() <= 0) {
        return false;
    }
    else if(request_.parse(readBuff_)) {    // 解析成功
        LOG_DEBUG("%s", request_.path().c_str());
        response_.init(srcDir, request_.path(), request_.isKeepAlive(), 200);
    } else {
        response_.init(srcDir, request_.path(), false, 400);
    }

    response_.makeResponse(writeBuff_); // 生成响应报文放入writeBuff_中
    // 响应头
    iov_[0].iov_base = const_cast<char*>(writeBuff_.peek());
    iov_[0].iov_len = writeBuff_.readableBytes();
    iovCnt_ = 1;

    // 文件
    if(response_.fileLen() > 0  && response_.file()) {
        iov_[1].iov_base = response_.file();
        iov_[1].iov_len = response_.fileLen();
        iovCnt_ = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response_.fileLen() , iovCnt_, writeBytesLength());
    return true;
}

