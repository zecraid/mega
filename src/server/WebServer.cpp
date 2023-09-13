#include "WebServer.h"

WebServer::WebServer() {
    Util::welcome();
    timeoutMS_ = 60000;
    isClose_ = false;
    timer_ = std::make_unique<HeapTimer>();
    threadpool_ = std::make_unique<ThreadPool>(std::thread::hardware_concurrency());
    epoller_ = std::make_unique<Epoller>();
}

void WebServer::init(uint16_t port,int logLevel, int logQueSize, const char *sqlLocal, uint16_t sqlPort, const char *sqlUser,
                     const char *sqlPwd, const char *dbName, int connPoolNum, const char *srcDir) {
    port_ = port;
    HttpConnection::srcDir = srcDir;
    HttpConnection::userCount = 0;
    SQLConnectionPool::instance()->init(sqlLocal, sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);
    Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
    initEventMode_(3);
    if(!initSocket_()){
        isClose_ = true;
    }
}

WebServer::~WebServer() {
    close(listenFd_);
    isClose_ = true;
    SQLConnectionPool::instance()->closePool();
}

void WebServer::initEventMode_(int trigMode) {
    listenFd_ = EPOLLRDHUP;
//    clientEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    clientEvent_ = EPOLLONESHOT;
    switch (trigMode)
    {
        case 0:
            break;
        case 1:
            clientEvent_ |= EPOLLET;
            break;
        case 2:
            listenEvent_ |= EPOLLET;
            break;
        case 3:
            listenEvent_ |= EPOLLET;
            clientEvent_ |= EPOLLET;
            break;
        default:
            listenEvent_ |= EPOLLET;
            clientEvent_ |= EPOLLET;
            break;
    }
    HttpConnection::isET = (clientEvent_ & EPOLLET);
}

void WebServer::start() {
    int timeMS = -1;  // timeout == -1 无事件将阻塞
    if(!isClose_) { LOG_INFO("Server Start ...SUCCESS"); }
    while (!isClose_){
        if(timeoutMS_ > 0) {
            timeMS = timer_->getNextTick();
        }
        int eventCnt = epoller_->wait(timeMS);
        for(int i = 0; i < eventCnt; i++) {
            int fd = epoller_->getEventFd(i);
            uint32_t events = epoller_->getEvents(i);
            if(fd == listenFd_) {
                dealListen_();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                closeConn_(&users_[fd]);
            }
            else if(events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                dealRead_(&users_[fd]);
            }
            else if(events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                dealWrite_(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void WebServer::sendError_(int fd, const char *info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

void WebServer::closeConn_(HttpConnection *client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->getFd());
    epoller_->delFd(client->getFd());
    client->close();
}

void WebServer::addClient_(int fd) {
    assert(fd > 0);
    users_[fd].init(fd);
    if(timeoutMS_ > 0) {
        timer_->add(fd, timeoutMS_, std::bind(&WebServer::closeConn_, this, &users_[fd]));
    }
    epoller_->addFd(fd, EPOLLIN | clientEvent_);
    setFdNonblock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].getFd());
}

void WebServer::dealListen_() {
    do {
        int fd = accept(listenFd_, nullptr, nullptr); // TODO:set nullptr ,timer 失效？
        if(fd <= 0) { return;}
        else if(HttpConnection::userCount >= MAX_FD) { // 用户数大于MAX_FD
            sendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        addClient_(fd);
    } while(listenEvent_ & EPOLLET);
}

void WebServer::dealRead_(HttpConnection *client) {
    assert(client);
    extentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::onRead_, this, client)); // 这是一个右值，bind将参数和函数绑定
}

void WebServer::dealWrite_(HttpConnection *client) {
    assert(client);
    extentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::onWrite_, this, client));
}

void WebServer::extentTime_(HttpConnection *client) {
    assert(client);
    if(timeoutMS_ > 0) { timer_->adjust(client->getFd(), timeoutMS_); }
}

void WebServer::onRead_(HttpConnection *client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno); // 读取数据到HttpConnection的read_buff_
    if(ret <= 0 && readErrno != EAGAIN) {   // 读异常
        closeConn_(client);
        return;
    }
    onProcess(client);
}

void WebServer::onProcess(HttpConnection *client) {
    if(client->process()) { // 读完将connEvent的事件转为EPOLLOUT
        epoller_->modFd(client->getFd(), clientEvent_ | EPOLLOUT);
    } else {
        epoller_->modFd(client->getFd(), clientEvent_ | EPOLLIN);
    }
}

void WebServer::onWrite_(HttpConnection *client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if(client->writeBytesLength() == 0) {
        if(client->isKeepAlive()) {
            epoller_->modFd(client->getFd(), clientEvent_ | EPOLLIN); // 回归换成监测读事件
            return;
        }
    } else if(ret < 0) {
        if(writeErrno == EAGAIN) {  // 缓冲区满了
            epoller_->modFd(client->getFd(), clientEvent_ | EPOLLOUT);
            return;
        }
    }
    closeConn_(client);
}


bool WebServer::initSocket_() {
    int ret;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0) {
        LOG_ERROR("Create socket error!", port_);
        return false;
    }

    int optval = 1;

    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }

    // 绑定
    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR("Bind Port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    // 监听
    ret = listen(listenFd_, 8);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    // 加入红黑树
    ret = epoller_->addFd(listenFd_,  listenEvent_ | EPOLLIN);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listenFd_);
        return false;
    }
    setFdNonblock(listenFd_);
    LOG_INFO("Server port:%d", port_);
    return true;
}

int WebServer::setFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}
