#include "Channel.h"

const int Channel::READ_EVENT = 1;
const int Channel::WRITE_EVENT = 2;
const int Channel::EXIT_EVENT = 4;
const int Channel::ET = 8;

Channel::Channel(EventLoop *loop, Socket *socket) :loop_(loop), socket_(socket){}

Channel::~Channel() {
    loop_->deleteChannel(this);
}

void Channel::handleEvent() {
    if(ready_events_ & READ_EVENT){
        read_callback_();
    }
    if(ready_events_ & WRITE_EVENT){
        write_callback_();
    }
    if(ready_events_ & EXIT_EVENT){
        exit_callback_();
    }
}

void Channel::enableRead() {
    listen_events_ |= READ_EVENT;
    loop_->updateChannel(this);
}

void Channel::enableWrite() {
    listen_events_ |= WRITE_EVENT;
    loop_->updateChannel(this);
}

void Channel::enableExit() {
    listen_events_ |= EXIT_EVENT;
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

void Channel::setReadyEvents(int event) {
    if(event & READ_EVENT){
        ready_events_ |= READ_EVENT;
    }
    if(event & WRITE_EVENT) {
        ready_events_ |= WRITE_EVENT;
    }
    if(event & EXIT_EVENT){
        ready_events_ |= EXIT_EVENT;
    }
    if(event & ET) {
        ready_events_ |= ET;
    }
}

void Channel::setReadCallback(std::function<void()> const &callback) {
    read_callback_ = callback;
}

void Channel::setWriteCallback(std::function<void()> const &callback) {
    write_callback_ = callback;
}

void Channel::setExitCallback(const std::function<void()> &callback) {
    exit_callback_ = callback;
}



