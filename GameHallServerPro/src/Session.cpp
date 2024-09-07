//
// Created by xmr on 2024/8/7.
//

#include "Session.h"

#include <functional>
#include "User.h"
#include "Room.h"
#include "RoomMgr.h"
#include "GameServer.h"

Session::Session()
{
    static uint64_t netId = 0;
    _netId = ++netId;
    _funcs["CS_Login"] = std::bind(&Session::CS_Login, this, std::placeholders::_1);
    _funcs["CS_Register"] = std::bind(&Session::CS_Register, this, std::placeholders::_1);
    _funcs["CS_Logout"] = std::bind(&Session::CS_Logout, this, std::placeholders::_1);
    _funcs["CS_SurePreemptLogin"] = std::bind(&Session::CS_SurePreemptLoginEvent, this, std::placeholders::_1);
}

Session::~Session()
{

}

int Session::OnNetMsg(const char *const buff, int len)
{
    if (len <= 2)return 0;
    uint16_t packSize = *((uint16_t *) buff);

    std::string msg(buff + 2, packSize - 2);

    cJSON *root = cJSON_Parse(msg.c_str());

    cJSON *cmd = cJSON_GetObjectItem(root, "cmd");//root 的附属对象 无须删除

    if (cmd->type == cJSON_String)
    {
        auto find = _funcs.find(cmd->valuestring);
        if (find != _funcs.end())
        {
            //收到指令
            LOG_INFO << "Session:收到指令 [" << cmd->valuestring << "]";
            find->second(root);
        }
        else if (_user!=nullptr)
        {
            _user->OnNetMsg(root);
        }
         else LOG_ERROR << "未找到相关指令 [" << cmd->valuestring << "]";
    }


    cJSON_Delete(root);
    return packSize;
}

void Session::OnDisconnect()
{
	LOG_INFO << "客户端关闭";
    if (_user != nullptr)
    {
    Room* room = _user->GetRoom();
    if (room != nullptr)
    {
        //服务端删除只有掉线用户所在房间
        int roomId = room->GetRoomId();
        GameServer::Ins()->ServerDeleteRoom(roomId);
    }
    GameServer::Ins()->RemoveUser(_user);
	_user = nullptr;
    }
	GameServer::Ins()->RemoveSession(this);
}

void Session::SendJSON(cJSON*root)
{
    const char *buf = cJSON_PrintUnformatted(root);
    int bufLen = strlen(buf);
    uint16_t len = bufLen + 2;
    Send((char *) &len, 2);
    Send(buf, bufLen);
    free((void *) buf);
}