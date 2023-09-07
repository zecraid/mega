#ifndef __EPOLL_H__
#define __EPOLL_H__
#include <sys/epoll.h>
#include <vector>
#include "Channel.h"

class Epoll {
public:
    Epoll();
    ~Epoll();

    std::vector<Channel*> poll(int timeout = -1) const;
    void updateChannel(Channel*) const; // 更新
    void deleteChannel(Channel*) const; // 将
private:
    int epfd_{1}; // 红黑树的epfd
    struct epoll_event *events_{nullptr};
};


#endif //!__EPOLL_H__
