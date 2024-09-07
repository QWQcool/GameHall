#pragma once
#include<iostream>
#include "ImguiAll.h"
#include "Protocol.h"

#include "GoBangGame.h"

class Player;

class GameFactory
{
public:
	static GameBase* CreateGame(const std::string& gameType)
	{
		if (gameType == u8"五子棋")
			return new GoBang();
		else
			return nullptr; // 或者抛出异常
	}
};

class Room
{
private:
	int _roomId;
	int _playerNum;
	int _playerMaxNum;
	std::string _roomName;
	std::string _roomGameType;
	int _roomOwnerId;
	Player** _players;
	int _isGameShow;
	GameBase* _game; //在游戏开始事件成功时再初始化
public:
	//和服务端不同
	Room(int _roomId, int _playerNum, int playerMaxNum, const std::string& roomName, const std::string& roomGameType, int roomOwnerId)
		: _roomId(_roomId), _playerNum(_playerNum), _playerMaxNum(playerMaxNum), _roomName(roomName), _roomGameType(roomGameType), _roomOwnerId(roomOwnerId)
	{
		_players = new Player * [_playerMaxNum];
		for (int i = 0; i < _playerMaxNum; i++)
		{
			_players[i] = nullptr;
		}
		_isGameShow = 0;
		_game = nullptr;
	}

	~Room()
	{
		if(_game)delete _game; // 释放游戏实例
		delete[] _players; // 释放玩家指针数组
	}

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
	GameBase* GetGameBase() const { if(_game)return _game; }
	int GetIsGameShow() const { return _isGameShow; }
	Player** GetPlayers() const { return _players; }

	// Setter methods
	void SetIsGameShow(int state) { _isGameShow = state; }
	void SetRoomName(const std::string& name) { _roomName = name; }
	void SetRoomGameType(const std::string& gameType) 
	{ 
		// 如果游戏类型改变，则需要重新创建游戏实例
		if (_roomGameType != gameType)
		{
			if(_game)
			delete _game;
			//_game = GameFactory::CreateGame(gameType);
			_roomGameType = gameType;
		}
	}
	void SetRoomOwner(int ownerId) { _roomOwnerId = ownerId; }
	void SetPlayerNum(int playerNum) { _playerNum = playerNum; }
	void SetPlayerMaxNum(int playerMaxNum) 
	{ 
		// 如果最大玩家数改变，需要重新分配玩家数组
		if (_playerMaxNum != playerMaxNum)
		{
			Player** newPlayers = new Player * [playerMaxNum];
			for (int i = 0; i < _playerMaxNum && i < playerMaxNum; i++)
			{
				newPlayers[i] = _players[i];
			}
			for (int i = _playerMaxNum; i < playerMaxNum; i++)
			{
				newPlayers[i] = nullptr;
			}
			delete[] _players;
			_players = newPlayers;
			_playerMaxNum = playerMaxNum;
		}
	}

	void ChangeRoomInfo(int playerNum, const std::string& roomName, int roomOwnerId)
	{
		_playerNum = playerNum;
		_roomName = roomName;
		_roomOwnerId = roomOwnerId;
	}

	void SetRoomInfo(int playerNum, int playerMaxNum, const std::string& name, const std::string& gameType, int ownerId)
	{
		_playerNum = playerNum;
		_playerMaxNum = playerMaxNum;
		_roomName = name;
		_roomGameType = gameType;
		_roomOwnerId = ownerId;
	}

	void AddPlayer() { _playerNum++; }
	void RemovePlayer() { if (_playerNum > 0) _playerNum--; }

	void ShowWidget();

	void Draw_LineGraph(int pos, ImVec2 graphChange, ImU32 color = IM_COL32(255, 0, 0, 255));
	void Draw_TextGraph(int pos, ImVec2 graphChange, Player* player);
	void ShowRoomWidget();

	void ClearGame()
	{
		if (_game)
		{
			_game->ClearGame();
			delete _game;
			_game = nullptr;
		}
	}

	bool JoinRoom(Player* player);

	bool ExitRoom(int exitUserId);

	bool GameStart();
};