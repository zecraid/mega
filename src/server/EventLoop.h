#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__
#include <functional>
#include <memory>
#include "Channel.h"
#include "Epoll.h"
class Epoll;
class Channel;
class EventLoop
{

public:
    EventLoop();
    ~EventLoop() = default;

    void loop();
    void updateChannel(Channel *ch);
    void deleteChannel(Channel *ch);

private:
    std::unique_ptr<Epoll> epoll_;
};


#endif