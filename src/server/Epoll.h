#ifndef __EPOLL_H__
#define __EPOLL_H__
#include <sys/epoll.h>
#include <vector>
#include "Channel.h"

class Epoll {
public:
    Epoll(); // 创建红黑树epfd
    ~Epoll();

    std::vector<Channel*> poll(int timeout = -1) const; // 执行epoll_wait，根据就绪事件（这里是EPOLLIN、EPOLLOUT...），绑定上Channel的标记位（READ_EVENT）
    void updateChannel(Channel* ch) const; // 更新
    void deleteChannel(Channel* ch) const; // 将
private:
    int epfd_{1}; // 红黑树的epfd
    struct epoll_event *events_{nullptr};
};


#endif //!__EPOLL_H__
