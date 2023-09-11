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
    bool initSocket_();
    void initEventMode_(int trigMode);
    void addClient_(int fd, sockaddr_in addr);

    void dealListen_();
    void dealWrite_(HttpConnection* client);
    void dealRead_(HttpConnection* client);

    void sendError_(int fd, const char*info);
    void extentTime_(HttpConnection* client);
    void closeConn_(HttpConnection* client);

    void onRead_(HttpConnection* client);
    void onWrite_(HttpConnection* client);
    void onProcess(HttpConnection* client);

    static int setFdNonblock(int fd);

private:

    static const int MAX_FD = 65536;
    int port_;
    int timeoutMS_;  /* 毫秒MS */
    bool isClose_;
    int listenFd_;
    char* srcDir_;

    uint32_t listenEvent_;  // 监听事件
    uint32_t connEvent_;    // 连接事件

    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConnection> users_;

};


#endif //!__WEBSERVER_H__
