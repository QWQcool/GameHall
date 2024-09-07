#include <iostream>
#include <XNet.h>
#include "EventLoopMgr.h"
#include "GameServer.h"
#include "DBMgr.h"
#include "Session.h"

#ifdef _WIN32
#pragma comment(lib, "XNet.lib")
#endif

int main() {
    //    system("chcp 65001");
    X::Socket::InitNetWork();

    GameServer::Ins()->Init();
    DBMgr::Ins()->Init();


    X::TcpAccept accept(EventLoopMgr::GetMainEL(), [](const X::NetPoint* const netPoint) {
        LOG_DEBUG << "客户端链接：" << netPoint->sock;
        Session* sock = new Session();
        sock->OnAccept(netPoint, EventLoopMgr::GetMainEL());
        GameServer::Ins()->AddSession(sock);
        });
    if (!accept.Listen(9999)) {
        LOG_ERROR << "监听端口失败";
        return 0;
    }
    LOG_INFO << "服务端 监听:0.0.0.0 端口:" << 9999;
    while (true) {
        EventLoopMgr::GetMainEL()->LoopOnce();
    }
    return 0;
}