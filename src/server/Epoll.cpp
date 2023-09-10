#include "Epoll.h"
#include "Util.h"
#include "Channel.h"
#include <unistd.h>
#include <string.h>

#define MAX_EVENTS 1000

Epoll::Epoll(){
    epfd_ = epoll_create1(0);
    Util::errif(epfd_ == -1, "epoll create error");
    events_ = new epoll_event[MAX_EVENTS];
    bzero(events_, sizeof(*events_) * MAX_EVENTS);
}

Epoll::~Epoll(){
    if(epfd_ != -1){
        close(epfd_);
        epfd_ = -1;
    }
    delete [] events_;
}

std::vector<Channel*> Epoll::poll(int timeout) const{
    std::vector<Channel*> active_channels;
    int re_event_nums = epoll_wait(epfd_, events_, MAX_EVENTS, timeout);
    Util::errif(re_event_nums == -1, "epoll wait error");
    for(int i = 0; i < re_event_nums; ++i){
        Channel *ch = (Channel *)events_[i].data.ptr;
        int events = events_[i].events;
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

void Epoll::updateChannel(Channel *channel) const{
    int sockfd = channel->getSocket()->getFd();
    struct epoll_event ev {};
    ev.data.ptr = channel;
    if (channel->getListenEvents() & Channel::READ_EVENT) {
        ev.events |= EPOLLIN | EPOLLPRI;
    }
    if (channel->getListenEvents() & Channel::WRITE_EVENT) {
        ev.events |= EPOLLOUT;
    }
    if (channel->getListenEvents() & Channel::ET) {
        ev.events |= EPOLLET;
    }
    if (!channel->getExist()) {
        Util::errif(epoll_ctl(epfd_, EPOLL_CTL_ADD, sockfd, &ev) == -1, "epoll add error");
        channel->setExist();
    } else {
        Util::errif(epoll_ctl(epfd_, EPOLL_CTL_MOD, sockfd, &ev) == -1, "epoll modify error");
    }
}

void Epoll::deleteChannel(Channel *channel) const{
    int sockfd = channel->getSocket()->getFd();
    Util::errif(epoll_ctl(epfd_, EPOLL_CTL_DEL, sockfd, nullptr) == -1, "epoll delete error");
    channel->setExist(false);
}

