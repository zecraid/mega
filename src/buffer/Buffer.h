#ifndef __BUFFER_H_
#define __BUFFER_H_
#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>

class Buffer {
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t writableBytes() const;
    size_t readableBytes() const;
    size_t prependableBytes() const;

    const char* peek() const; // 返回可读区域的首指针
    void ensureWriteable(size_t len); // 确保即将append的数据是否有足够的空间来接受数据
    void moveWritePos(size_t len); // 移动writePos
    void moveReadPos(size_t len); // 移动readPos
    void moveReadPosToEnd(const char *end); // 移动readPos到end位置

    void retrieveAll(); // 清空buffer，双坐标清0
    std::string RetrieveAllToStr(); // 取出buffer内容，然后清空buffer，双坐标清0

    const char *beginWritePtrConst() const; // 可写位置指针
    char *beginWritePtr(); // 可写位置指针

    void append(const std::string& str);
    void append(const char* str, size_t len);
    void append(const void* data, size_t len);
    void append(const Buffer& buff);

    ssize_t readFd(int fd, int* Errno);
    ssize_t writeFd(int fd, int* Errno);

private:
    char* beginPtr_();
    const char *beginPtr_() const;
    void makeSpace_(size_t len);

    std::vector<char> buffer_;
    std::atomic<std::size_t> readPos_;  // 读下标
    std::atomic<std::size_t> writePos_; // 写下标
};
#endif //!__BUFFER_H_
