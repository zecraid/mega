#include "Epoll.h"
#include "Util.h"
#include "Channel.h"
#include <unistd.h>
#include <string.h>

#define MAX_EVENTS 1000

Epoll::Epoll() {
    epfd_ = epoll_create1(0);
    Util::errif(epfd_ == -1, "epoll create error");
    events_ = new epoll_event[MAX_EVENTS];
    bzero(events_, sizeof(*events_) * MAX_EVENTS);
}

Epoll::~Epoll() {
    if(epfd_ != -1){
        close(epfd_);
        epfd_ = -1;
    }
    delete [] events_;
}

std::vector<Channel *> Epoll::poll(int timeout) const {
    std::vector<Channel*> active_channels;
    int ready_event_nums = epoll_wait(epfd_, events_, MAX_EVENTS, timeout);
    Util::errif(ready_event_nums == -1, "epoll wait error");
    for(int i = 0; i < ready_event_nums; ++i){
        Channel *ch = (Channel *)events_[i].data.ptr; // 将就绪事件的ptr强制转换为Channel类
        int events = events_[i].events;
        // 根据Epoll标记为绑定Channel专属标记位
        if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
            ch->setReadyEvents(Channel::EXIT_EVENT);
        }
        if (events & EPOLLIN) {
            ch->setReadyEvents(Channel::READ_EVENT);
        }
        if (events & EPOLLOUT) {
            ch->setReadyEvents(Channel::WRITE_EVENT);
        }
        if (events & EPOLLET) {
            ch->setReadyEvents(Channel::ET);
        }
        active_channels.push_back(ch);
    }
    return active_channels;
}

void Epoll::updateChannel(Channel *ch) const {
    int sockfd = ch->getSocket()->getFd();
    struct epoll_event ev {};
    ev.data.ptr = ch;
    if (ch->getListenEvents() & Channel::READ_EVENT) {
        ev.events |= EPOLLIN | EPOLLPRI;
    }
    if (ch->getListenEvents() & Channel::WRITE_EVENT) {
        ev.events |= EPOLLOUT;
    }
    if (ch->getListenEvents() & Channel::ET) {
        ev.events |= EPOLLET;
    }
//    if(ch->getListenEvents() & Channel::EXIT_EVENT){
//        ev.events |=
//    }
    if (!ch->getExist()) {
        Util::errif(epoll_ctl(epfd_, EPOLL_CTL_ADD, sockfd, &ev) == -1, "epoll add error");
        ch->setExist();
    } else {
        Util::errif(epoll_ctl(epfd_, EPOLL_CTL_MOD, sockfd, &ev) == -1, "epoll modify error");
    }
}

void Epoll::deleteChannel(Channel *ch) const {
    int sockfd = ch->getSocket()->getFd();
    Util::errif(epoll_ctl(epfd_, EPOLL_CTL_DEL, sockfd, nullptr) == -1, "epoll delete error");
    ch->setExist(false);
}