//
// Created by xmr on 2024/8/6.
//

#include "GameServer.h"
#include "Session.h"
#include "EventLoopMgr.h"
#include "User.h"
#include "RoomMgr.h"

void GameServer::Init() 
{
    EventLoopMgr::GetMainEL()->Init([this](void *ptr) {
        OnPost((IGameCell *) ptr);
    });
}

void GameServer::AddSession(Session *session) 
{
    _sessions.insert(std::pair<uint64_t, Session *>(session->GetNetId(), session));
}

void GameServer::RemoveSession(Session *session) {
    _sessions.erase(session->GetNetId());
    delete session;
}

Session *GameServer::FindNetIdSession(uint64_t id) 
{
    auto find = _sessions.find(id);
    if (find == _sessions.end()) return nullptr;

    return find->second;

}

void GameServer::OnPost(IGameCell *cell) {
    cell->Exec();
    cell->Release();
}

void GameServer::Post(IGameCell *cell) {
    EventLoopMgr::GetMainEL()->Post(cell);
}

void GameServer::AddUser(User *user) {
    _user4uids.insert(std::pair<uint64_t, User *>(user->GetUserId(), user));
    _user4usernames.insert(std::pair<std::string, User *>(user->GetUsername(), user));
}

void GameServer::RemoveUser(User *user) {
    _user4uids.erase(user->GetUserId());
    _user4usernames.erase(user->GetUsername());
    delete user;
}

void GameServer::RemoveUserByUserId(int userId)
{
    auto find = _user4uids.find(userId);
    if (find != _user4uids.end())
        _user4uids.erase(find);
}

User *GameServer::FindUser(uint64_t uid) {
    auto find = _user4uids.find(uid);
    if (find == _user4uids.end())
        return nullptr;
    return find->second;
}

User *GameServer::FindUser(std::string username) {
    auto find = _user4usernames.find(username);
    if (find == _user4usernames.end())
        return nullptr;
    return find->second;
}

void GameServer::ServerDeleteRoom(int roomId)
{
    for (auto iter : _sessions)
    {
        User * user = iter.second->GetUser();
        if (user != nullptr)
        {
            int userId = user->GetUserId();
            user->ServerDeleteRoom(userId, roomId);
			Room* room = RoomMgr::GetInstance().FindRoom(roomId);

			if (room != nullptr)
			{
				RoomMgr::GetInstance().DeleteRoom(roomId);
			}
        }
    }
}
