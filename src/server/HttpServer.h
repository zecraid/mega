#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include "common.h"
#include "HttpConnection.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "../pool/ThreadPool.h"
#include "../http/HttpConnection.h"
#include "../log/Log.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <map>
#include <vector>
class EventLoop;
class Socket;
class Acceptor;
class Connection;
class ThreadPool;
class Logger;
class HttpServer
{
public:
    HttpServer(const char* ip, uint16_t port);
    ~HttpServer() = default;
    void init(Logger::Level logLevel, int logQueSize,
              const char* sqlLocal, uint16_t sqlPort, const char* sqlUser, const  char* sqlPwd,
              const char* dbName, int connPoolNum, const char *srcDir);
    void start();

    ST newConnection(int fd);
    ST deleteConnection(int fd);

    void onConnect(std::function<void(Connection *)> fn);
    void onRecv(std::function<void(Connection *)> fn);
    void newConnect(std::function<void(Connection *)> fn);
private:
    std::unique_ptr<EventLoop> main_reactor_;
    std::unique_ptr<Acceptor> acceptor_;

    std::unordered_map<int, std::unique_ptr<Connection>> connections_;
    std::vector<std::unique_ptr<EventLoop>> sub_reactors_;

    std::unique_ptr<ThreadPool> thread_pool_;

    std::function<void(Connection *)> on_connect_;
    std::function<void(Connection *)> on_recv_;
    std::function<void(Connection *)> new_connect_;
};

#endif //__TCPSERVER_H__