//
// Created by xmr on 2024/8/6.
//

#ifndef GAMESERVER_EVENTLOOPMGR_H
#define GAMESERVER_EVENTLOOPMGR_H

#include <XNet.h>

class EventLoopMgr
{
private:
    X::EventLoop _mainEventLoop;
    X::EventLoop _dbEventLoop;
public:
    static EventLoopMgr *Ins()
    {
        static EventLoopMgr ins;
        return &ins;
    }

    static X::EventLoop *GetMainEL()
    {
        return &Ins()->_mainEventLoop;
    }

    static X::EventLoop *GetDBEL()
    {
        return &Ins()->_dbEventLoop;
    }
};


#endif //GAMESERVER_EVENTLOOPMGR_H
