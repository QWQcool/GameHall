//
// Created by xmr on 2024/8/15.
//

#ifndef GAMESERVER_ROOMMGR_H
#define GAMESERVER_ROOMMGR_H

#include <iostream>
#include <string>
#include <map>

class Room;
class User;

class RoomMgr
{
private:
	std::map<int, Room*> _rooms;
	RoomMgr() {}; // 私有构造函数
	static RoomMgr* _instance; // 静态指针实例
	~RoomMgr() {};
	RoomMgr(const RoomMgr&) = delete;
	RoomMgr& operator=(const RoomMgr&) = delete;
public:
	std::map<int, Room*> GetRoomListMap() { return _rooms; }
	static RoomMgr& GetInstance(); // 获取单例对象的静态方法
	//返回值是房间id
	int CreateRoom(int playerMaxNum, const std::string& roomName, const std::string& roomGameType, int roomOwnerId);

	bool JoinRoomEvent(int roomId, User* player);

	bool DeleteRoom(int roomId);

	Room* FindRoom(int roomId) const;

	// 列出所有房间
	void ListRooms() const;
	// 其他方法
};


#endif //GAMESERVER_ROOMMGR_H
