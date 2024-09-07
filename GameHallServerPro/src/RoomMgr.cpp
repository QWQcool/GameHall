//
// Created by xmr on 2024/8/15.
//

#include "RoomMgr.h"
#include "Room.h"
#include "User.h"

RoomMgr* RoomMgr::_instance = nullptr;

RoomMgr& RoomMgr::GetInstance()
{
	if (!_instance)
	{
		_instance = new RoomMgr();
	}
	return *_instance;
}

//返回值是房间ID
int RoomMgr::CreateRoom(int playerMaxNum, const std::string& roomName, const std::string& roomGameType, int roomOwnerId)
{
	auto room = new Room(playerMaxNum, roomName, roomGameType, roomOwnerId);
	_rooms.insert(std::make_pair(room->GetRoomId(), room));
	return room->GetRoomId();
}

bool RoomMgr::JoinRoomEvent(int roomId, User* player)
{
	for (auto iter = _rooms.begin(); iter != _rooms.end(); iter++)
	{
		if (iter->second->GetRoomId() == roomId)
		{
			//iter->second->AddPlayer();
			//JoinRoom里面增加
			iter->second->JoinRoom(player);
			return true;
		}
	}
	return false;
}

bool RoomMgr::DeleteRoom(int roomId)
{
	auto it = _rooms.find(roomId);
	if (it != _rooms.end())
	{
		delete it->second; // 释放 Room 对象的内存
		_rooms.erase(it); // 从 map 中移除该元素
		return true;
	}
	return false;
}

Room* RoomMgr::FindRoom(int roomId) const
{
	auto it = _rooms.find(roomId);
	if (it != _rooms.end())
	{
		return it->second;
	}
	return nullptr;
}

// 列出所有房间
void RoomMgr::ListRooms() const
{
	for (const auto& room : _rooms)
	{
		std::cout << "Room ID: " << room.second->GetRoomId() << std::endl;
		std::cout << "Room Name: " << room.second->GetRoomName() << std::endl;
		std::cout << "Room OwnerID: " << room.second->GetRoomOwner() << std::endl;
		std::cout << "Room Type: " << room.second->GetRoomGameType() << std::endl;
		std::cout << "Player Count: " << room.second->GetPlayerNum() << "/" << room.second->GetPlayerMaxNum() << std::endl;
		std::cout << "---------------------" << std::endl;
	}
}