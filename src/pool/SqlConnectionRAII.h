#ifndef __SQLCONNECTIONRAII_H__
#define __SQLCONNECTIONRAII_H__


#include "SqlConnectionPool.h"

/* 资源在对象构造初始化 资源在对象析构时释放*/
class SQLConnectionRAII{
public:
    SQLConnectionRAII(MYSQL** sql, SQLConnectionPool *connpool) {
        assert(connpool);
        *sql = connpool->getConnection();
        sql_ = *sql;
        connpool_ = connpool;
    }
    ~SQLConnectionRAII() {
        if(sql_) {
            connpool_->freeConnection(sql_);
        }
    }
private:
    MYSQL *sql_;
    SQLConnectionPool* connpool_;
};

#endif // !_SQLCONNECTIONRAII_H_