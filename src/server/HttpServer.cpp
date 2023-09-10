#include "HttpServer.h"
#include "EventLoop.h"
#include "../log/Log.h"
#include "../pool/SQLConnectionPool.h"
#include "Util.h"
#include <cassert>

HttpServer::HttpServer(const char* ip, uint16_t port) {
    Util::welcome();
    main_reactor_ = std::make_unique<EventLoop>();
    acceptor_ = std::make_unique<Acceptor>(main_reactor_.get(), ip, port);
    std::function<void(int)> cb = std::bind(&HttpServer::newConnection, this, std::placeholders::_1);
    acceptor_->setNewConnectionCallback(cb);

    unsigned int size = std::thread::hardware_concurrency();
    thread_pool_ = std::make_unique<ThreadPool>(size);

    for(size_t i = 0; i < size; ++i){
        std::unique_ptr<EventLoop> sub_reactor = std::make_unique<EventLoop>();
        sub_reactors_.push_back(std::move(sub_reactor));
    }
}

void HttpServer::start() {
    for(size_t i = 0; i< sub_reactors_.size(); ++i){
        std::function<void()> sub_loop = std::bind(&EventLoop::loop, sub_reactors_[i].get());
        thread_pool_->add(std::move(sub_loop));
    }
    info("HTTP Server Start...");
    main_reactor_->loop();
}

void HttpServer::init(Logger::Level logLevel, int logQueSize,
                      const char* sqlLocal, uint16_t sqlPort, const char* sqlUser, const  char* sqlPwd,
                      const char* dbName, int connPoolNum, const char *srcDir) {
    HttpConnection::srcDir = srcDir;

    printf("========== SERVER INIT ==========\n");
    Log::Instance()->init(logLevel,"./log", ".log", logQueSize);
    printf("Log system: open\n");
    printf("Log level: %s\n",Logger::instance()->getLevelStr());
    printf("Log directory: %s\n","./log");
    printf("Log mode: %s\n",logQueSize == 0? "normal":"async");

    printf("Resource directory: %s\n",srcDir);
    printf("Database ip: %s\n",sqlLocal);
    printf("Database user: %s\n",sqlUser);
    printf("Database name: %s\n",dbName);
    printf("SQL connection pool num: %d\n", connPoolNum);
    printf("ThreadPool num: %d\n", std::thread::hardware_concurrency());
    printf("==========  INIT COMPLETE  ==========\n");
    SQLConnectionPool::instance()->init(sqlLocal, sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);  // 连接池单例的初始化

}

ST HttpServer::newConnection(int fd) {
    assert(fd != -1);
    uint64_t random = fd % sub_reactors_.size();

    std::unique_ptr<HttpConnection> conn = std::make_unique<HttpConnection>(fd, sub_reactors_[random].get());
    std::function<void(int)> cb = std::bind(&HttpServer::deleteConnection, this, std::placeholders::_1);

    conn->setDeleteConnection(cb);
    conn->setOnRecv(on_recv_);

    connections_[fd] = std::move(conn);
    if(on_connect_){
        on_connect_(connections_[fd].get());
    }
    return ST_SUCCESS;
}

ST HttpServer::deleteConnection(int fd) {
    auto it = connections_.find(fd);
    assert(it != connections_.end());
    connections_.erase(fd);
    return ST_SUCCESS;
}

void HttpServer::onConnect(std::function<void(HttpConnection *)> fn) {
    on_connect_ = std::move(fn);
}

void HttpServer::onRecv(std::function<void(HttpConnection *)> fn) {
    on_recv_ = std::move(fn);
}

void HttpServer::newConnect(std::function<void(HttpConnection *)> fn) {
    new_connect_ = std::move(fn);
}