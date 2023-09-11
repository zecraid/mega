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

    bool AddFd(int fd, uint32_t events);
    bool ModFd(int fd, uint32_t events);
    bool DelFd(int fd);

    int Wait(int timeoutMs = -1);
    int GetEventFd(size_t i) const;
    uint32_t GetEvents(size_t i) const;

private:
    int epfd_;
    std::vector<struct epoll_event> events_;

};

#endif //!__EPOLLER_H__
