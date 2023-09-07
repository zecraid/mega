#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include "../pool/ThreadPool.h"
#include <vector>
#include <thread>

EventLoop::EventLoop() {
    epoll_ = std::make_unique<Epoll>();
}

void EventLoop::loop() {
    while (true){
        for(Channel *active_channel : epoll_->poll()){
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
