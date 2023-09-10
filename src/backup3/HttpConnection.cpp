#include "HttpConnection.h"
const char * HttpConnection::srcDir;

HttpConnection::HttpConnection(int fd, EventLoop *loop) {
    LOG_INFO("创建HttpConnection,fd = %d",fd);
    socket_ = std::make_unique<Socket>(fd);
    if(loop != nullptr){
        channel_ = std::make_unique<Channel>(loop, socket_.get());
        std::function<void()> read_cb = std::bind(&HttpConnection::readRequest, this);
        channel_->setReadCallback(read_cb);
        std::function<void()> write_cb = std::bind(&HttpConnection::sendResponse, this);
        channel_->setWriteCallback(write_cb);

        channel_->enableRead(); // TODO：设置回调函数
        channel_->enableET();
    }
    write_buf_ = std::make_unique<Buffer>();
    read_buf_ = std::make_unique<Buffer>();
    write_buf_->retrieveAll();
    read_buf_->retrieveAll();

    request_ = std::make_unique<HttpRequest>();
    response_ = std::make_unique<HttpResponse>();
    state_ = ConnState::CONNECTED;
}

HttpConnection::~HttpConnection() {
    response_->unmapFile();
}


ssize_t HttpConnection::read(int *saveErrno) {
    ssize_t len = -1;
    do {
        len = read_buf_->readFd(getFd(), saveErrno);
        LOG_INFO("读,len=%d", len);
        if (len <= 0) {
            break;
        }
    } while (true); // ET:边沿触发要一次性全部读出
    return len;
}

ssize_t HttpConnection::write(int *saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(getFd(), iov_, iovCnt_);   // 将iov的内容写到fd中
        if(len <= 0) {
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
            write_buf_->moveReadPos(len);
        }
    } while(writeBytesLength() > 10240);
    return len;
}

bool HttpConnection::processRequest() {
    request_->init();
    if(read_buf_->readableBytes() <= 0){
        LOG_ERROR("read_buff EMPTY");
        return false;
    } else if(request_->parse(read_buf_.get())){
        LOG_DEBUG("Request Parse Success:%s %s", request_->method().c_str(),request_->path().c_str());
        response_->init(srcDir, request_->path(), request_->isKeepAlive(), 200);
    } else {
        response_->init(srcDir, request_->path(), false, 400);
    }
    response_->makeResponse(write_buf_.get());

    iov_[0].iov_base = const_cast<char*>(write_buf_->peek());
    iov_[0].iov_len = write_buf_->readableBytes();
    iovCnt_ = 1;

    if(response_->fileLen() > 0 && response_->file()){
        iov_[1].iov_base = response_->file();
        iov_[1].iov_len = response_->fileLen();
        iovCnt_ = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response_->fileLen(), iovCnt_, writeBytesLength());
    return true;
}

void HttpConnection::readRequest() {
    read_buf_->retrieveAll();
    int ret = -1;
    int readErrno = 0; // TODO:读完写，写完读
    ret = read(&readErrno);
    if(ret <=0 && readErrno != EAGAIN){
        closeConnection();
        return;
    }
    channel_->setReadyEvents(Channel::WRITE_EVENT);
}

void HttpConnection::sendResponse() {
    int saveErrno = -1;
    if(processRequest()){
        HttpConnection::write(&saveErrno);
    } else {
        LOG_ERROR("processRequest error");
    }
    write_buf_->retrieveAll();
    read_buf_->retrieveAll();
    channel_->setReadyEvents(Channel::READ_EVENT);
}

void HttpConnection::setCloseConnectionCallback(const std::function<void(int)> &fn) {
    close_connection_ = std::move(fn);
}

void HttpConnection::closeConnection() {
    LOG_INFO("client fd = %d close connection", socket_->getFd());
    close_connection_(socket_->getFd());
}

int HttpConnection::writeBytesLength() {
    return iov_[0].iov_len + iov_[1].iov_len;
}

bool HttpConnection::isKeepAlive() {
    return request_->isKeepAlive();
}

int HttpConnection::getFd() const {
    return socket_->getFd();
}

std::string HttpConnection::getAddr() const {
    return socket_->getAddr();
}
