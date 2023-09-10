#ifndef __EPOLL_H__
#define __EPOLL_H__
#include <sys/epoll.h>
#include <vector>
#include "Channel.h"
class Channel;
class Epoll
{

public:
    Epoll();
    ~Epoll();

    std::vector<Channel*> poll(int timeout = -1) const;
    void updateChannel(Channel*) const;
    void deleteChannel(Channel*) const;
private:
    int epfd_{1};
    struct epoll_event *events_{nullptr};
};

#endif //__EPOLL_H__