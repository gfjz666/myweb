#pragma once
#include "sqlconnpool.h"
/* 资源在对象构造初始化 资源在对象析构时释放*/
class SqlConnRAII
{
private:
    MYSQL *sql_;
    SqlConnPool *connpool_;

public:
    SqlConnRAII(MYSQL **sql, SqlConnPool *connpool)
    {
        assert(connpool);
        *sql = connpool->GetConn();
        sql_ = *sql;
        connpool_ = connpool;
    }
    ~SqlConnRAII()
    {
        if (sql_)
            connpool_->FreeConn(sql_);
    }
};