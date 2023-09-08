#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "State.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "HttpConnection.h"
#include "../pool/ThreadPool.h"
#include "../timer/HeapTimer.h"

class WebServer {
public:
    WebServer(const char* ip, uint16_t port);
    ~WebServer() = default;
    void init(int logLevel, int logQueSize,
              const char* sqlLocal, uint16_t sqlPort, const char* sqlUser, const  char* sqlPwd,
              const char* dbName, int connPoolNum, const char *srcDir);

    void start();

    ST newConnection(int fd);
    ST deleteConnection(int fd);

public:
    static std::atomic<int> userCount;  // 原子，支持锁

private:
    std::unique_ptr<EventLoop> main_reactor_;
    std::vector<std::unique_ptr<EventLoop>> sub_reactors_;
    std::unique_ptr<Acceptor> acceptor_;

    std::unique_ptr<HeapTimer> timer_;
    int timeoutMS_; // 超时事件
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unordered_map<int, std::unique_ptr<HttpConnection>> connections_;
};


#endif //!__WEBSERVER_H__
