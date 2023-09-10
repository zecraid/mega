#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include <vector>

EventLoop::EventLoop(bool timer_on = true) {
    timer_on_ = true;
    epoll_ = std::make_unique<Epoll>();
    if(timer_on_){
        timeoutMS_ = 60000; // 超时时间为60s
        timer_ = std::make_unique<HeapTimer>();
    }
}

void EventLoop::loop() {
    int timeMS = -1; // epoll wait timeout == -1 无事件将阻塞
    while(true){
        if(timer_on_){
            if(timeoutMS_ > 0){
                timeMS = timer_->getNextTick();
            }
        }

        for(Channel *active_channel : epoll_->poll(timeMS)){
            active_channel->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel *ch) {
    epoll_->updateChannel(ch);
}

void EventLoop::deleteChannel(Channel *ch) {
    epoll_->deleteChannel(ch);
}