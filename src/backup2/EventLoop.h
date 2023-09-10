#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__
#include <functional>
#include <memory>
#include "Channel.h"
#include "Epoll.h"
#include "../timer/HeapTimer.h"
class Epoll;
class Channel;
class HeapTimer;
class EventLoop
{
public:
    EventLoop(bool timer_on); //
    ~EventLoop() = default;

    void loop();
    void updateChannel(Channel *ch);
    void deleteChannel(Channel *ch);

private:
    std::unique_ptr<Epoll> epoll_;
    std::unique_ptr<HeapTimer> timer_;
    bool timer_on_;
    int timeoutMS_;
};


#endif