#pragma once
#include<iostream>
#include<unordered_map>
#include<functional>
#include "cJSON.h"

class GameBase;
class User;

class Room
{
private:
	friend class RoomManager;
	int _roomId;
	int _playerNum;
	int _playerMaxNum;
	std::string _roomName;
	std::string _roomGameType;
	int _roomOwnerId;
	User** _players;
	GameBase* _game;
	std::unordered_map<std::string, std::function<void(cJSON* root)>> _funcs;
public:
	void OnNetMsg(cJSON* root);

	void SendJSON(cJSON* root,int userId);

	void CS_MovePos(cJSON* root);

	void SC_MovePos(int userId, int roomId, int oldPos, int newPos);

	void CS_ReadyPos(cJSON* root);

	void SC_ReadyPos(int userId, int roomId, int pos, int readyState);

	void CS_AgainGame(cJSON* root);

	void SC_AgainGame(int userId, int roomId, const std::string& gameType, int againState);

	void CS_GamePlay(cJSON* root);

	void SC_GamePlay(int userId, int roomId);

	void CS_GoBangDown(cJSON* root);

	void SC_GoBangDown(int userId,int roomId,int xPos,int yPos,int playerIndex);

	Room(int playerMaxNum, const std::string& roomName, const std::string& roomGameType, int roomOwnerId);

	~Room()
	{}

	int GetNetId(int userId);
	// Getter methods
	int GetRoomId() const
	{
		return _roomId;
	}
	int GetPlayerNum() const { return _playerNum; }
	int GetPlayerMaxNum() const { return _playerMaxNum; }
	std::string GetRoomName() const { return _roomName; }
	std::string GetRoomGameType() const { return _roomGameType; }
	int GetRoomOwner() const { return _roomOwnerId; }
	GameBase* GetGameBase() const { return _game; }
	User** GetPlayers() const { return _players; }

	// Setter methods
	void SetRoomName(const std::string& name) { _roomName = name; }
	void SetRoomGameType(const std::string& gameType) { _roomGameType = gameType; }
	void SetRoomOwner(int owner) { _roomOwnerId = owner; }
	void SetPlayerNum(int playerNum) { _playerNum = playerNum; }
	void SetPlayerMaxNum(int playerMaxNum) { _playerMaxNum = playerMaxNum; }

	void SetRoomInfo(int playerNum, int playerMaxNum, const std::string& name, const std::string& gameType, int ownerId)
	{
		_playerNum = playerNum;
		_playerMaxNum = playerMaxNum;
		_roomName = name;
		_roomGameType = gameType;
		_roomOwnerId = ownerId;
	}

	bool IsEmpty() const { return _playerNum == 0; }
	bool OnlySelf(int userId) const;

	void AddPlayerNum() { _playerNum++; }
	void RemovePlayerNum() { if (_playerNum > 0) _playerNum--; }
	void ClearGame();

	bool JoinRoom(User* player);

	int FindEmptyPos();

	void BroadcastPlayerJoin(int pos);

	void SendRoomInfo(int pos);

};