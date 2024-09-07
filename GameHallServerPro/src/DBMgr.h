//
// Created by xmr on 2024/8/7.
//

#ifndef GAMESERVER_DBMGR_H
#define GAMESERVER_DBMGR_H

#include "EventLoopMgr.h"
#include <DBSqlite.h>

class IDBCell
{
public:
    virtual ~IDBCell()
    {}

    virtual void DBExec(X::DB::DBSqlite *pSqlite) = 0;

    virtual void Release()
    {
        delete this;
    }
};

class DBMgr
{
private:
    X::DB::DBSqlite _sqlite;
public:
    static DBMgr *Ins()
    {
        static DBMgr ins;
        return &ins;
    }

    void Init();

    static void Post(IDBCell *cell)
    {
        EventLoopMgr::GetDBEL()->Post(cell);
    }

private:
    void OnPost(void *ptr);
};


#endif //GAMESERVER_DBMGR_H
