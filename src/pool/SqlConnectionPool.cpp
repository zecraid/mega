#include "SqlConnectionPool.h"

SQLConnectionPool *SQLConnectionPool::instance()
{
    static SQLConnectionPool pool;
    return &pool;
}

void SQLConnectionPool::init(const char *host, uint16_t port,
                             const char *user, const char *pwd,
                             const char *dbName, int connSize = 10)
{
    assert(connSize > 0);
    for (int i = 0; i < connSize; i++)
    {
        MYSQL *conn = nullptr;
        conn = mysql_init(conn);
        if (!conn)
        {
            LOG_ERROR("MySql init error!");
            assert(conn);
        }
        conn = mysql_real_connect(conn, host, user, pwd, dbName, port, nullptr, 0);
        if (!conn)
        {
            LOG_ERROR("MySql Connect error!");
        }
        connQue_.emplace(conn);
    }
    MAX_CONN_ = connSize;
    sem_init(&semId_, 0, MAX_CONN_);
}

MYSQL *SQLConnectionPool::getConnection()
{
    MYSQL *conn = nullptr;
    if (connQue_.empty())
    {
        LOG_WARN("SqlConnPool busy!");
        return nullptr;
    }
    sem_wait(&semId_); // -1
    std::lock_guard<std::mutex> locker(mtx_);
    conn = connQue_.front();
    connQue_.pop();
    return conn;
}

// 存入连接池，实际上没有关闭
void SQLConnectionPool::freeConnection(MYSQL *conn)
{
    assert(conn);
    std::lock_guard<std::mutex> locker(mtx_);
    connQue_.push(conn);
    sem_post(&semId_); // +1
}

void SQLConnectionPool::closePool()
{
    std::lock_guard<std::mutex> locker(mtx_);
    while (!connQue_.empty())
    {
        auto conn = connQue_.front();
        connQue_.pop();
        mysql_close(conn);
    }
    mysql_library_end();
}

int SQLConnectionPool::getFreeConnCount()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return connQue_.size();
}