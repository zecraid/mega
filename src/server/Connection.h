//#ifndef __CONNECTION_H__
//#define __CONNECTION_H__
//#include <functional>
//#include <memory>
//#include "common.h"
//#include "EventLoop.h"
//#include "../buffer/Buffer.h"
//#include "Socket.h"
//#include "Channel.h"
//
//class Connection {
//public:
//    enum State{
//        INVALID = 0,
//        CONNECTING,
//        CONNECTED,
//        CLOSED,
//    };
//    Connection(int fd, EventLoop *loop);
//    ~Connection() = default;
//
//    void setDeleteConnection(std::function<void(int)> const &fn);
//    void setOnConnect(std::function<void(Connection *)> const &fn);
//    void setOnRecv(std::function<void(Connection *)> const &fn);
//
//    State state() const;
//    Socket *getSocket() const;
//
//    void setSendBuf(const char *str);
//    Buffer *getReadBuf();
//    Buffer *getSendBuf();
//
//    ST read();
//    ST write();
//    ST send(std::string msg);
//
//    void close();
//
//    void onConnect(std::function<void()> fn);
//    void onMessage(std::function<void()> fn);
//
//private:
//    void bussiness_();
//    ST readNonBlocking_();
//    ST writeNonBlocking_();
//    ST readBlocking_();
//    ST writeBlocking_();
//
//private:
//    std::unique_ptr<Socket> socket_;
//    std::unique_ptr<Channel> channel_;
//
//    State state_;
//    std::unique_ptr<Buffer> read_buf_;
//    std::unique_ptr<Buffer> send_buf_;
//
//    std::function<void(int)> delete_connectioin_;
//    std::function<void(Connection *)> on_recv_;
//
//};
//
//
//#endif //REBUILD_V3_CONNECTION_H
