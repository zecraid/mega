#ifndef __EPOLLER_H__
#define __EPOLLER_H__
#include <sys/epoll.h> //epoll_ctl()
#include <unistd.h> // close()
#include <assert.h> // close()
#include <vector>
#include <errno.h>

class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);
    ~Epoller();

    bool addFd(int fd, uint32_t events); // 将FD和监听事件挂载到红黑树上
    bool modFd(int fd, uint32_t events); // 修改红黑树上fd的监听事件
    bool delFd(int fd); // 将fd从红黑数上删除

    int wait(int timeoutMs = -1); // epoll_wait
    int getEventFd(size_t i) const; // 获取event[index] 的就绪fd
    uint32_t getEvents(size_t i) const; // 获取event[index] 的就绪事件

private:
    int epfd_;
    std::vector<struct epoll_event> ready_events_;

};

#endif //!__EPOLLER_H__
