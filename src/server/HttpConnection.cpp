#include "HttpConnection.h"
const char* HttpConnection::srcDir;
std::atomic<int> HttpConnection::userCount;
bool HttpConnection::isET;

HttpConnection::HttpConnection() {
    fd_ = -1;
    isClose_ = true;
}

HttpConnection::~HttpConnection() {
    close();
}

void HttpConnection::init(int sockFd) {
    assert(sockFd > 0);
    userCount++;
    fd_ = sockFd;
    response_ = std::make_unique<HttpResponse>();
    request_ = std::make_unique<HttpRequest>();
    read_buff_ = std::make_unique<Buffer>();
    write_buff_ = std::make_unique<Buffer>();
    read_buff_->retrieveAll();
    write_buff_->retrieveAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s) in, userCount:%d", fd_, getAddress().c_str(), (int)userCount);
}

void HttpConnection::close() {
    response_->unmapFile();
    if(isClose_ == false){
        isClose_ = true;
        userCount--;
        LOG_INFO("Client[%d](%s) in, userCount:%d", fd_, getAddress().c_str(), (int)userCount);
        ::close(fd_);
    }
}

int HttpConnection::getFd() const {
    return fd_;
}

std::string HttpConnection::getAddress() const {
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

ssize_t HttpConnection::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = read_buff_->readFd(fd_, saveErrno);
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
                write_buff_->retrieveAll();
                iov_[0].iov_len = 0;
            }
        }
        else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            write_buff_->moveReadPos(len);
        }
    } while(isET || writeBytesLength() > 10240);
    return len;
}

bool HttpConnection::process() {
    request_->init();
    if(read_buff_->readableBytes() <= 0) {
        return false;
    }
    else if(request_->parse(read_buff_.get())) {    // 解析成功
        response_->init(srcDir, request_->path(), request_->isKeepAlive(), 200);
    } else {
        response_->init(srcDir, request_->path(), false, 400);
    }
    response_->makeResponse(write_buff_.get()); // 生成响应报文放入write_buff_中
    // 响应头
    iov_[0].iov_base = const_cast<char*>(write_buff_->peek());
    iov_[0].iov_len = write_buff_->readableBytes();
    iovCnt_ = 1;

    // 文件
    if(response_->fileLen() > 0  && response_->file()) {
        iov_[1].iov_base = response_->file();
        iov_[1].iov_len = response_->fileLen();
        iovCnt_ = 2;
    }
    return true;
}

