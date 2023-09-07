#include "Channel.h"
#include <unistd.h>

const int Channel::READ_EVENT = 1; // 二进制：1
const int Channel::WRITE_EVENT = 2; // 二进制：10
const int Channel::ET = 4; // 二进制：100

Channel::Channel(EventLoop *loop, Socket *socket) :loop_(loop), socket_(socket){}

Channel::~Channel() {
    loop_->deleteChannel(this); // 将自己从红黑树上摘下来
}

// 根据标记位分配执行不同的回调函数
void Channel::handleEvent() {
    if(ready_events_ & READ_EVENT){
        read_callback_();
    }
    if(ready_events_ & WRITE_EVENT){
        write_callback_();
    }
}

// 添加上可读标记位，然后传给loop，再传给Epoll，
// Epoll会将READ_EVENT、WRITE_EVENT、ET转换为对应的EPOLLIN、EPOLLOUT、EPOLLET等标记位后并挂载到红黑树上
void Channel::enableRead() {
    listen_events_ |= READ_EVENT;
    loop_->updateChannel(this);
}

void Channel::enableWrite() {
    listen_events_ |= WRITE_EVENT;
    loop_->updateChannel(this);
}

void Channel::enableET() {
    listen_events_ |= ET;
    loop_->updateChannel(this);
}

Socket *Channel::getSocket() {
    return socket_;
}

int Channel::getListenEvents() {
    return listen_events_;
}

int Channel::getReadyEvents() {
    return ready_events_;
}

bool Channel::getExist() {
    return exist_;
}

void Channel::setExist(bool exist) {
    exist_ = exist;
}

// 传入包含READ_EVENT、WRITE_EVENT、ET的标记为组合event
void Channel::setReadyEvents(int event) {
    if(event & READ_EVENT){
        ready_events_ |= READ_EVENT;
    }
    if(event & WRITE_EVENT) {
        ready_events_ |= WRITE_EVENT;
    }
    if(event & ET) {
        ready_events_ |= ET;
    }
}

// 设置读回调，这里设置的回调函数会在handleEvent中执行
void Channel::setReadCallback(std::function<void()> const &callback) {
    read_callback_ = callback;
}

// 设置写回调，这里设置的回调函数会在handleEvent中执行
void Channel::setWriteCallback(std::function<void()> const &callback) {
    write_callback_ = callback;
}
