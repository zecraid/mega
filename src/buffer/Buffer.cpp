#include "Buffer.h"

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0){}

// 可写的长度
size_t Buffer::writableBytes() const {
    return buffer_.size() - writePos_;
}

// 可读的长度
size_t Buffer::readableBytes() const {
    return writePos_ - readPos_;
}

// 可预留空间（已读完的长度）
size_t Buffer::prependableBytes() const {
    return readPos_;
}

const char *Buffer::peek() const {
    return &buffer_[readPos_];
}

void Buffer::ensureWriteable(size_t len) {
    if(len > writableBytes()){
        makeSpace_(len);
    }
    assert(len <= writableBytes());
}

void Buffer::moveWritePos(size_t len) {
    writePos_ += len;
}

void Buffer::moveReadPos(size_t len) {
    readPos_ += len;
}

void Buffer::moveReadPosToEnd(const char *end) {
    assert(peek() <= end);
    moveReadPos(end - peek());
}

void Buffer::retrieveAll() {
    bzero(&buffer_[0], buffer_.size()); // 覆盖原本数据
    readPos_ = writePos_ = 0;
}

std::string Buffer::RetrieveAllToStr() {
    std::string str(peek(), readableBytes());
    RetrieveAll();
    return str;
}

const char *Buffer::beginWritePtrConst() const {
    return &buffer_[writePos_];
}

char *Buffer::beginWritePtr() {
    return &buffer_[writePos_];
}

void Buffer::append(const char *str, size_t len) {
    assert(str);
    ensureWriteable(len); // 确保可写的长度
    std::copy(str, str + len, beginWritePtr()); //将str放到写下标开始的地方
    moveWritePos(len);
}

void Buffer::append(const std::string &str) {
    append(str.c_str(), str.size());
}

void Buffer::append(const void *data, size_t len) {
    append(static_cast<const char*>(data), len);
}

void Buffer::append(const Buffer &buff) {
    append(buff.peek(), buff.readableBytes());
}

ssize_t Buffer::readFd(int fd, int *Errno) {
    char buff[65535]; // 栈区
    struct iovec iov[2];
    size_t writable = writableBytes(); // 先记录能写多少
    // 分散读
    iov[0].iov_base = beginWritePtr();
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    ssize_t len = readv(fd, iov, 2); // 分散读，读到buffer和char buff[65535]上
    if(len < 0){
        *Errno = errno;
    } else if(static_cast<size_t>(len) <= writable){ // 若len小于writable，说明写区可以容纳len
        moveWritePos(len);
    } else {
        writePos_ = buffer_.size(); // 写区存满
        append(buff, static_cast<size_t>(len - writable)); // 剩余的长度append
    }
    return len;
}

ssize_t Buffer::writeFd(int fd, int *Errno) {
    ssize_t len = write(fd, peek(), readableBytes());
    if(len < 0){
        *Errno = errno;
        return len;
    }
    moveReadPos(len); // 读到fd之后，移动readPos
    return len;
}

char *Buffer::beginPtr_() {
    return &buffer_[0];
}

const char *Buffer::beginWritePtrConst() const {
    return &buffer_[0];
}

void Buffer::makeSpace_(size_t len) {
    if(writableBytes() + prependableBytes() < len){
        buffer_.resize(writePos_ + len + 1);
    } else {
        size_t readable = readableBytes();
        std::copy(beginPtr_() + readPos_, beginPtr_() + writePos_, beginPtr_());
        readPos_ = 0;
        writePos_ = readable;
        assert(readable == readableBytes());
    }
}

