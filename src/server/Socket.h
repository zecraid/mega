#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "State.h"
#include <string>

class Socket {
public:
    Socket();
    Socket(int fd);
    ~Socket();

    void setFd(int fd); // 设置Socket的文件描述符
    int getFd() const; // 获取文件描述符
    std::string getAddr(); // 反向通过fd_拿到拼接后的地址 IP:PORT

    ST create(); // 创建ListenFd
    ST bind(const char* ip, uint16_t port) const; // 绑定服务器地址结构
    ST listen() const; // 绑定最大事件数目
    ST accept(int &client_fd) const; // 监听并创建client_fd,client_fd最后服务于Connection
    ST connect(const char *ip, uint16_t port) const; // 连接服务器
    ST setNonBlocking(); // fd设置非阻塞
    bool isNonBlocking() const; // 判断fd是否为非阻塞
    ST setReuseAddr(); // 设置端口复用（TIME_WAIT期间也能占用端口）

private:
    int fd_; // 每一个Socket都有一个文件描述符
};

#endif //!__SOCKET_H__
