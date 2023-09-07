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

    void loop(); // 进入无限遍历epoll_wait返回的active_channel
    void updateChannel(Channel *ch); // 更新Channel（挂载红黑树）
    void deleteChannel(Channel *ch); // 删除Channel（从红黑树上删除）

private:
    std::unique_ptr<Epoll> epoll_;
};

#endif