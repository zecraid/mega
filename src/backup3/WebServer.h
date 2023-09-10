#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__
#include <functional>
#include <unordered_map>
#include <vector>
#include <map>
#include <vector>
#include "State.h"
#include "HttpConnection.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "Util.h"
#include "../pool/ThreadPool.h"
#include "../log/Log.h"
#include "../timer/HeapTimer.h"
class EventLoop;
class Socket;
class Acceptor;
class HttpConnection;
class ThreadPool;
class Log;
class HeapTimer;
class WebServer{
public:
    WebServer(const char* ip, uint16_t port);
    ~WebServer() = default;

    void init(int logLevel, int logQueSize,
              const char* sqlLocal, uint16_t sqlPort, const char* sqlUser, const  char* sqlPwd,
              const char* dbName, int connPoolNum, const char *srcDir);
    void start();

    void newConnection(int fd); // Acceptor回调函数
    void closeConnection(int fd);

public:
    static std::atomic<int> userCount;  // 用户数量，原子，支持锁

private:
    std::unique_ptr<EventLoop> main_reactor_;
    std::vector<std::unique_ptr<EventLoop>> sub_reactors_;

    std::unique_ptr<Acceptor> acceptor_;

    static const int MAX_CONN_ = 65536;
    std::unordered_map<int, std::unique_ptr<HttpConnection>> connections_;

    std::unique_ptr<ThreadPool> thread_pool_;
};

#endif //!__WEBSERVER_H__