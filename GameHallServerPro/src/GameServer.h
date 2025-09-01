//
// Created by xmr on 2024/8/6.
//

#ifndef GAMESERVER_GAMESERVER_H
#define GAMESERVER_GAMESERVER_H

#include <cstdint>          // 标准整数类型
#include <unordered_map>    // 哈希表容器
#include <string>           // 字符串类
class Session;              // 前向声明Session类

// 游戏逻辑单元接口
class IGameCell {
public:
	virtual ~IGameCell() {}             // 虚析构函数

	virtual void Exec() = 0;            // 执行逻辑

	virtual void Release() { delete this; }; // 释放资源
};

class User; // 前向声明User类

// 游戏服务器主类
class GameServer {
private:
	// 会话管理
	std::unordered_map<uint64_t, Session*> _sessions;      // 按网络ID存储会话

	// 用户管理
	std::unordered_map<std::string, User*> _user4usernames; // 按用户名存储用户
	std::unordered_map<uint64_t, User*> _user4uids;         // 按用户ID存储用户

public:
	// 单例模式访问
	static GameServer* Ins() {
		static GameServer ins;
		return &ins;
	}

	// 异步逻辑投递
	void Post(IGameCell* cell);

	// 初始化
	void Init();

	// 会话管理
	void AddSession(Session* session);             // 添加会话
	void RemoveSession(Session* session);          // 移除会话
	Session* FindNetIdSession(uint64_t id);        // 查找会话

	// 用户管理
	void AddUser(User* user);                      // 添加用户
	void RemoveUser(User* user);                   // 移除用户
	void RemoveUserByUserId(int userId);           // 按用户ID移除用户
	User* FindUser(uint64_t uid);                  // 按用户ID查找用户
	User* FindUser(std::string username);          // 按用户名查找用户

	// 房间管理
	void ServerDeleteRoom(int roomId);             // 删除房间

private:
	void OnPost(IGameCell* cell);                  // 处理投递的逻辑单元
};

#endif //GAMESERVER_GAMESERVER_H
