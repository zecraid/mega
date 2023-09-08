#include "HttpConnection.h"
const char* HttpConnetion::srcDir;
bool HttpConnetion::isET;

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
    request_ = std::make_unique<HttpRequest>();
    response_ = std::make_unique<HttpResponse>();
    state_ = State::CONNECTED;
}

HttpConnection::~HttpConnection() {
    close();
}

void HttpConnection::close() {
    response_->unmapFile();
    if(!isClose_){
        isClose_ = true;
        close(fd)_;
        LOG_INFO("Client[%d](%s) quit", fd_, getAddr().c_str());
    }
}

void HttpConnection::init(int fd, EventLoop *loop) {
    assert(fd > 0);
    socket_->setFd(fd);
    read_buf_->retrieveAll();
    write_buf_->retrieveAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s) quit ", fd_, getAddr().c_str());
}

ssize_t HttpConnection::read(int *saveErrno) {
    ssize_t len = -1;
    do {
        len = read_buf_->readFd(getFd(), saveErrno);
        if(len <= 0){
            *saveErrno = errno;
            break;
        }
    } while (isET);
    return len;
}

ssize_t HttpConnection::write(int *saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(getFd(), iov_, iovCnt_); // 将iov的内容写到fd中
        if(len <= 0){
            *saveErrno = errno;
            break;
        }
        if(iov_[0].iov_len + iov_[1].iov_len  == 0) { break; } /* 传输结束 */
        else if(static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len) {
                write_buf_->retrieveAll();
                iov_[0].iov_len = 0;
            }
        }
        else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            write_buf_->moveReadPos(len)
        }
    } while (isET || writeBytesLength() > 10240);
    return len;
}

bool HttpConnection::process() {
    request_->init();
    if(read_buf_->readableBytes() <= 0){
        return false;
    } else if(request_->parse(read_buf_)){
        LOG_DEBUG("%s", request_.path().c_str());
        response_->init(srcDir, request_.path(), request_.IsKeepAlive(), 200);
    } else {
        response_->init(srcDir, request_.path(), false, 400);
    }
    response_->makeResponse(write_buf_);// 生成响应报文放入writeBuff_中
    // 响应头
    iov_[0].iov_base = const_cast<char*>(write_buf_->peek());
    iov_[0].iov_len = write_buf_->readableBytes();
    iovCnt_ = 1;

    // 文件
    if(response_->fileLen() > 0 && response_->file()){
        iov_[1].iov_base = response_->file();
        iov_[1].iov_len = response_->fileLen();
        iovCnt_ = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response_->fileLen() , iovCnt_, writeBytesLength());
    return true;
}

void HttpConnection::setCloseConnectionCallback(const std::function<void(int)> &fn) {
    close_connection_ = std::move(fn);
}

void HttpConnection::closeConnection() {
    close_connection_(socket_->getFd());
}

void HttpConnection::setRequestRecvCallback() {
//    std::function<void()> cb = std::bind(&HttpConnection::read, this);
    std::function<void()> cb = [](){
        int saveErrno = -1;
        read(&saveErrno);
        if(process()){
            write(&saveErrno);
        }
    };
    channel_->setReadCallback(cb);
}

//void HttpConnection::setResponseSendCallback() {
//    std::function<void()> cb = [](){
//        int saveErrno = -1;
//
//        if(process()){ // 处理接受到的数据
//            write();
//        }
//    };
//    channel_->setWriteCallback(cb);
//}

int HttpConnection::getFd() const {
    return socket_->getFd();
}

std::string HttpConnection::getAddr() const {
    return socket_->getAddr();
}

bool HttpConnection::isKeepAlive() const {
    return request_->isKeepAlive();
}

int HttpConnection::writeBytesLength() {
    return iov_[0].iov_len + iov_[1].iov_len;
}
