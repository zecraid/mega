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

#include "Epoller.h"
#include "HttpConnection.h"
#include "Util.h"
#include "../timer/HeapTimer.h"
#include "../log/Log.h"
#include "../pool/SqlConnectionPool.h"
#include "../pool/ThreadPool.h"


class WebServer {
public:
    WebServer(const char* ip, uint16_t port);
    ~WebServer();

    void init(int logLevel, int logQueSize,
              const char* sqlLocal, uint16_t sqlPort, const char* sqlUser, const  char* sqlPwd,
              const char* dbName, int connPoolNum, const char *srcDir);
    void start();

private:
    bool initSocket_(); // 创建ListenFd，挂载红黑树，设置非阻塞
    void initEventMode_(int trigMode); // 初始化listenEvent和connEvent
    void addClient_(int fd, sockaddr_in addr); // 将新连接添加集合中

    void dealListen_(); // 处理ListenFd发生的事件（创建ClientFd）
    void dealWrite_(HttpConnection* client); // 处理写
    void dealRead_(HttpConnection* client); // 处理读

    void sendError_(int fd, const char*info); // 发送错误信息
    void extentTime_(HttpConnection* client); // 如果有新的读写操作，重置定时器超时事件
    void closeConn_(HttpConnection* client); // 关闭连接

    void onRead_(HttpConnection* client); // 处理读
    void onWrite_(HttpConnection* client); // 处理写
    void onProcess(HttpConnection* client); // 读写EPOLL转换

    static int setFdNonblock(int fd);

private:

    static const int MAX_FD = 65536;
    int port_; // 开启端口
    int timeoutMS_; // 设置HttpConnection的连接时长
    bool isClose_; // 启动标志位
    int listenFd_;

    uint32_t listenEvent_;  // 监听事件
    uint32_t clientEvent_;    // 连接事件

    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConnection> users_;
};


#endif //!__WEBSERVER_H__
