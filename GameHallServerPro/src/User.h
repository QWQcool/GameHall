//
// Created by xmr on 2024/8/12.
//

#ifndef GAMESERVER_USER_H
#define GAMESERVER_USER_H

#include <string>
#include <functional>
#include <unordered_map>

#include "cJSON.h"

class Session;
class Room;

struct UserInfo {
    uint64_t uid = 0;
    std::string username = "";
    std::string password = "";
    std::string nickname = "";
};

class User 
{
private:
    UserInfo _userInfo;
    Session *_session;
    Room* _room;
    std::unordered_map<std::string, std::function<void(cJSON* root)>> _funcs;
    bool _isReady;
public:
    User();

    void Init(UserInfo userInfo) { _userInfo = userInfo; }

    void OnNetMsg(cJSON* root);

    User* GetUser() { return this; }
    const UserInfo &GetUserInfo() { return _userInfo; }
    uint64_t GetUserId() { return _userInfo.uid; }
    std::string GetUsername() { return _userInfo.username; }
    std::string GetNickname(){return _userInfo.nickname;}
    Session *GetSession() { return _session; }
    bool GetIsReady() const { return _isReady; }
    Room* GetRoom() { return _room; }

    void SetSession(Session *session) { _session = session; }
    void SetReady(int readyState) { readyState == 1 ? _isReady = true : false; }
    void SetRoom(Room* room) { _room = room; }

    uint64_t GetNetId();

    void SendJSON(cJSON* root);

    void CS_CreateRoom(cJSON* root);

    void SC_CreateRoom(const std::string& roomName, const std::string& roomGameType, int roomOwnerId);

    void CS_RefreshRoomList(cJSON* root);

    void SC_RefreshRoomList();

    void CS_JoinRoom(cJSON* root);

    void SC_JoinRoom(int userId, int roomId);

    void CS_ExitRoom(cJSON* root);

    void SC_ExitRoom(int userId, int roomId);

    void CS_ReturnRoom(cJSON* root);

    void SC_ReturnRoom(int userId, int roomId,int Operator, const std::string& gameType);

    void ServerDeleteRoom(int userId, int roomId);

    void CS_DeleteRoom(cJSON* root);

    void SC_DeleteRoom(int userId, int roomId);

};


#endif //GAMESERVER_USER_H
