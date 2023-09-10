#include "WebServer.h"

WebServer::WebServer(const char *ip, uint16_t port) {
    Util::welcome();
    main_reactor_ = std::make_unique<EventLoop>(false); // main_reactor不开启timer
    acceptor_ = std::make_unique<Acceptor>(main_reactor_.get(), ip, port);
    std::function<void(int)> cb = std::bind(&WebServer::newConnection, this, std::placeholders::_1);
    acceptor_->setNewConnectionCallback(cb);

    unsigned int size = std::thread::hardware_concurrency();
    thread_pool_ = std::make_unique<ThreadPool>(size);

    for(size_t i = 0; i < size; i++){
        std::unique_ptr<EventLoop> sub = std::make_unique<EventLoop>(true);
        sub_reactors_.push_back(std::move(sub));
    }
}

void WebServer::init(int logLevel, int logQueSize, const char *sqlLocal, uint16_t sqlPort, const char *sqlUser,
                     const char *sqlPwd, const char *dbName, int connPoolNum, const char *srcDir) {
    printf("========== SERVER INIT ==========\n");
    Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
    printf("Log Level:%d(%s)\n",logLevel, Log::Instance()->getLevelStr());
    printf("Log directory: %s\n","./log");
    printf("Log mode: %s\n",logQueSize == 0? "normal":"async");

    printf("Resource directory: %s\n",srcDir);
    HttpConnection::srcDir = srcDir;
    printf("Database IP: %s\n",sqlLocal);
    printf("Database user: %s\n",sqlUser);
    printf("Database name: %s\n",dbName);
    printf("SQL connection pool num: %d\n", connPoolNum);
    printf("ThreadPool num: %d\n", std::thread::hardware_concurrency());
    SQLConnectionPool::instance()->init(sqlLocal, sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);
    printf("==========  INIT COMPLETE  ==========\n");
}

void WebServer::start() {
    for(size_t i = 0; i < sub_reactors_.size(); i++){
        std::function<void()> sub_loop = std::bind(&EventLoop::loop, sub_reactors_[i].get());
        thread_pool_->add(std::move(sub_loop));
    }
    LOG_INFO("Web Server Start...SUCCESS");
    main_reactor_->loop();
}

void WebServer::newConnection(int fd) {
    assert(fd != -1);
    LOG_INFO("new client Comein");
//    uint16_t random = fd % sub_reactors_.size();
//    std::unique_ptr<HttpConnection> conn = std::make_unique<HttpConnection>(fd, sub_reactors_[random].get());
//    std::function<void(int)> cb = std::bind(&WebServer::closeConnection, this, std::placeholders::_1);
//    conn->setCloseConnectionCallback(cb);

//    connections_[fd] = std::move(conn); // err
    LOG_INFO("new client Add fd = %d",fd);
//    return ST_SUCCESS;
}

void WebServer::closeConnection(int fd) {
//    auto it = connections_.find(fd);
//    assert(it != connections_.end());
//    connections_.erase(fd);
//    return ST_SUCCESS;
}
