//
// Created by xmr on 2024/8/12.
//

#include "User.h"
#include "Session.h"
#include "Room.h"

User::User() : _userInfo() 
{
    _session = nullptr;
	_room = nullptr;
	_isReady = false;
    _funcs["CS_CreateRoom"] = std::bind(&User::CS_CreateRoom, this, std::placeholders::_1);
    _funcs["CS_RefreshRoomList"] = std::bind(&User::CS_RefreshRoomList, this, std::placeholders::_1);
    _funcs["CS_JoinRoom"] = std::bind(&User::CS_JoinRoom, this, std::placeholders::_1);
    _funcs["CS_ExitRoom"] = std::bind(&User::CS_ExitRoom, this, std::placeholders::_1);
}

void User::OnNetMsg(cJSON* root)
{
    try
    {
        auto cmd = cJSON_GetObjectItem(root, "cmd");
        auto func = _funcs.find(cmd->valuestring);
        if (func != _funcs.end())
        {
			//收到指令
            LOG_INFO << "User:收到指令 [" << cmd->valuestring << "]";
            func->second(root);
        }
        else if (_room != nullptr)
        {
            _room->OnNetMsg(root);
        }
        else
        {
            LOG_DEBUG<< "User::OnNetMsg func == nullptr";
        }
    }
    catch (std::exception& e)
    {
        LOG_DEBUG<< "User::OnNetMsg e:"<<e.what();
    }
}


uint64_t User::GetNetId()
{
    if (_session != nullptr)return _session->GetNetId();
    else
    {
        LOG_DEBUG<< "User::GetNetId _session == nullptr";
        return -1;
    }
}

void User::SendJSON(cJSON* root)
{
    if (_session != nullptr) _session->SendJSON(root);
    else
    {
        LOG_DEBUG<< "User::SendJSON _session == nullptr";
    }
}
