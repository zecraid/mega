#ifndef __SQLCONNECTIONPOOL_H__
#define __SQLCONNECTIONPOOL_H__

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/Log.h"

class SQLConnectionPool {
public:
    static SQLConnectionPool *instance();
    MYSQL *getConnection();
    void freeConnection(MYSQL *conn);
    int getFreeConnCount();

    void init(const char* host, uint16_t port,
              const char* user,const char* pwd,
              const char* dbName, int connSize);
    void closePool();
private:
    SQLConnectionPool() = default;
    ~SQLConnectionPool() { closePool(); }

    int MAX_CONN_;

    std::queue<MYSQL *> connQue_;
    std::mutex mtx_;
    sem_t semId_;
};



#endif //__SQLCONNECTIONPOOL_H__