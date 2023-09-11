#include "Epoller.h"

Epoller::Epoller(int maxEvent) {
    epfd_ = epoll_create(512);
    events_ = maxEvent;
    assert(epfd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller() {
    close(epfd_);
}

bool Epoller::AddFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::ModFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::DelFd(int fd) {
    if(fd < 0) return false;
    return 0 == epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, 0);
}

// 返回就绪事件数量
int Epoller::Wait(int timeoutMs) {
    return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
}

// 获取事件文件描述符
int Epoller::GetEventFd(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd;
}

// 获取事件属性
uint32_t Epoller::GetEvents(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].events;
}