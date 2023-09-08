#include "WebServer.h"
WebServer::WebServer(const char *ip, uint16_t port) {
    Util::welcome();
    main_reactor_ = std::make_unique<EventLoop>();
    acceptor_ = std::make_unique<Acceptor>(main_reactor_.get(), ip, port);
    std::function<void(int)> cb = std::bind(&WebServer::newConnection, this, std::placeholders::_1);
    acceptor_->setNewConnectionCallback(cb);

    unsigned int size = std::thread::hardware_concurrency();
    thread_pool_ = std::make_unique<ThreadPool>(size);

//    timer_ = std::make_unique<HeapTimer>();

    for(size_t i = 0; i < size; i++){
        std::unique_ptr<EventLoop> sub_reactor = std::make_unique<EventLoop>();
        sub_reactors_.push_back(std::move(sub_reactor));
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
//    HttpConnection::srcDir = srcDir;
//    HttpConnection::isET = true;
    printf("Database IP: %s\n",sqlLocal);
    printf("Database user: %s\n",sqlUser);
    printf("Database name: %s\n",dbName);
    printf("SQL connection pool num: %d\n", connPoolNum);
    SQLConnectionPool::instance()->init(sqlLocal, sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);  // 连接池单例的初始化
    printf("ThreadPool num: %d\n", std::thread::hardware_concurrency());
    printf("==========  INIT COMPLETE  ==========\n");
}

void WebServer::start() {
    for(size_t i = 0; i < sub_reactors_.size(); i++){
        std::function<void()> sub_loop = std::bind(&EventLoop::loop, sub_reactors_[i].get());
        thread_pool_->addTask(sub_loop);
    }
    LOG_INFO("Web Server Start...SUCCESS");
    main_reactor_->loop();
}

ST WebServer::newConnection(int fd) {
    assert(fd != -1);
    uint16_t random = fd % sub_reactors_.size();
    std::unique_ptr<HttpConnection> hconn = std::make_unique<HttpConnection>(fd, sub_reactors_[random].get());
    LOG_INFO("get new connection:fd = %d , sub_reactor = %d",fd, random);
    hconn->setRecvCallback(); // 设置channel的读回调函数用于接受Request请求

//    hconn->setResponseSendCallback(); // 设置channel的写回调函数用于处理Request请求组装
    connections_[fd] = std::move(hconn);
    return ST_SUCCESS;
}