//
// Created by xmr on 2024/8/6.
//

#include "GameServer.h"
#include "Session.h"
#include "EventLoopMgr.h"  // 事件循环管理
#include "User.h"          // 用户类
#include "RoomMgr.h"       // 房间管理

// 初始化游戏服务器
void GameServer::Init() {
	// 初始化主事件循环，设置回调函数
	EventLoopMgr::GetMainEL()->Init([this](void* ptr) 
		{
		OnPost((IGameCell*)ptr);
		});
}

// 添加会话
void GameServer::AddSession(Session* session) 
{
	// 将会话添加到映射表中，以网络ID为键
	_sessions.insert(std::pair<uint64_t, Session*>(session->GetNetId(), session));
}

// 移除会话
void GameServer::RemoveSession(Session* session) 
{
	// 从映射表中移除会话
	_sessions.erase(session->GetNetId());
	// 删除会话对象
	delete session;
}

// 查找会话
Session* GameServer::FindNetIdSession(uint64_t id) 
{
	auto find = _sessions.find(id);
	if (find == _sessions.end())
		return nullptr;
	return find->second;
}

// 处理投递的逻辑单元
void GameServer::OnPost(IGameCell* cell) {
	cell->Exec();    // 执行逻辑
	cell->Release(); // 释放资源
}

// 投递逻辑单元到事件循环
void GameServer::Post(IGameCell* cell) {
	EventLoopMgr::GetMainEL()->Post(cell);
}

// 添加用户
void GameServer::AddUser(User* user) {
	// 按用户ID存储
	_user4uids.insert(std::pair<uint64_t, User*>(user->GetUserId(), user));
	// 按用户名存储
	_user4usernames.insert(std::pair<std::string, User*>(user->GetUsername(), user));
}

// 移除用户
void GameServer::RemoveUser(User* user) {
	// 从用户ID映射表中移除
	_user4uids.erase(user->GetUserId());
	// 从用户名映射表中移除
	_user4usernames.erase(user->GetUsername());
	// 删除用户对象
	delete user;
}

// 按用户ID移除用户
void GameServer::RemoveUserByUserId(int userId) {
	auto find = _user4uids.find(userId);
	if (find != _user4uids.end())
		_user4uids.erase(find);
}

// 按用户ID查找用户
User* GameServer::FindUser(uint64_t uid) {
	auto find = _user4uids.find(uid);
	if (find == _user4uids.end())
		return nullptr;
	return find->second;
}

// 按用户名查找用户
User* GameServer::FindUser(std::string username) {
	auto find = _user4usernames.find(username);
	if (find == _user4usernames.end())
		return nullptr;
	return find->second;
}

// 删除房间
void GameServer::ServerDeleteRoom(int roomId) {
	// 遍历所有会话
	for (auto iter : _sessions) {
		User* user = iter.second->GetUser();
		if (user != nullptr) {
			int userId = user->GetUserId();
			// 通知用户删除房间
			user->ServerDeleteRoom(userId, roomId);

			// 查找房间
			Room* room = RoomMgr::GetInstance().FindRoom(roomId);
			if (room != nullptr) {
				// 从房间管理器中删除房间
				RoomMgr::GetInstance().DeleteRoom(roomId);
			}
		}
	}
}
