#ifndef __CHANNEL_H__
#define __CHANNEL_H__
#include <sys/epoll.h>
#include <functional>
#include "EventLoop.h"
#include "Socket.h"
class EventLoop;
class Socket;
class Channel {
public:
    Channel(EventLoop *loop, Socket *socket);
    ~Channel();

    void handleEvent(); // 位与读写标记，执行读写回调函数
    void enableRead(); // 标记读，标记完会同步到红黑树上
    void enableWrite(); // 标记写，标记完会同步到红黑树上
    void enableET(); // 标记边沿出发，标记完会同步到红黑树上

    Socket* getSocket(); // 获取Socket
    int getListenEvents(); // 获取监听事件（这里的事件是Channel专属事件：READ_EVENT、WRITE_EVENT、ET）
    int getReadyEvents(); // 获取就绪事件（这里的事件是Channel专属事件：READ_EVENT、WRITE_EVENT、ET）
    bool getExist(); // 获取存在标记位
    void setExist(bool exist = true); // 设置存在标记位

    void setReadyEvents(int event); // 设置就绪事件（这里的事件是Channel专属事件：READ_EVENT、WRITE_EVENT、ET）
    void setReadCallback(std::function<void()> const &callback); // 设置读回调
    void setWriteCallback(std::function<void()> const &callback); // 设置写回调

public:
    // Channel专属【二进制】标记，直接判断三大事件类型，这里不再使用EPOLL标记了
    static const int READ_EVENT; // 标记有读事件
    static const int WRITE_EVENT; // 标记有写事件
    static const int ET; // 标记边沿触发

private:
    EventLoop *loop_; // 反应堆（主/从）
    Socket *socket_; // 一个Channel也匹配一个Socket
    int listen_events_{0}; // 监听事件
    int ready_events_{0}; // 就绪事件
    bool exist_{false}; // 存在标记位
    std::function<void()> read_callback_; // 读回调
    std::function<void()> write_callback_; // 写回调


};

#endif //!__CHANNEL_H__
