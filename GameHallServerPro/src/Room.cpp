#include "Room.h"
#include "User.h"
#include "Session.h"
#include "GameBase.hpp"

Room::Room(int playerMaxNum, const std::string& roomName, const std::string& roomGameType, int roomOwnerId):_playerNum(0), _playerMaxNum(playerMaxNum), _roomName(roomName), _roomGameType(roomGameType), _roomOwnerId(roomOwnerId)
{
	static int roomId = 0;
	_roomId = ++roomId;

	_players = new User * [_playerMaxNum];
	for (int i = 0; i < _playerMaxNum; i++)
	{
		_players[i] = nullptr;
	}
	//_game = GameFactory::CreateGame(_roomGameType);//服务端应该在请求游戏开始时初始化游戏
	_funcs["CS_MovePos"] = std::bind(&Room::CS_MovePos, this, std::placeholders::_1);
	_funcs["CS_ReadyPos"] = std::bind(&Room::CS_ReadyPos, this, std::placeholders::_1);
	_funcs["CS_GamePlay"] = std::bind(&Room::CS_GamePlay, this, std::placeholders::_1);
	_funcs["CS_GoBangDown"] = std::bind(&Room::CS_GoBangDown, this, std::placeholders::_1);
	_funcs["CS_AgainGame"] = std::bind(&Room::CS_AgainGame, this, std::placeholders::_1);
}

int Room::GetNetId(int userId)
{
	for (int i=0;i<_playerMaxNum;i++)
	{
		if (_players[i]!=nullptr&&_players[i]->GetUserId() == userId)
			return _players[i]->GetNetId();
	}
	return -1;
}

void Room::OnNetMsg(cJSON* root)
{
	const char* cmd = cJSON_GetObjectItem(root, "cmd")->valuestring;
	//TODO
	try
	{
		auto cmd = cJSON_GetObjectItem(root, "cmd");
		auto func = _funcs.find(cmd->valuestring);
		if (func != _funcs.end())
		{
			//收到指令
			LOG_INFO << "Room:收到指令 [" << cmd->valuestring << "]";
			func->second(root);
		}
		else
		{
			LOG_DEBUG << "Room::OnNetMsg func == nullptr";
		}
	}
	catch (std::exception& e)
	{
		LOG_DEBUG << "User::OnNetMsg e:" << e.what();
	}
}

void Room::SendJSON(cJSON* root,int userId)
{
	for (int i = 0; i < _playerMaxNum; i++)
	{
		if (_players[i]->GetUserId() == userId)
		{
			_players[i]->SendJSON(root);
			break;
		}
	}
}


bool Room::OnlySelf(int userId) const
{
	for (int i = 0; i < _playerMaxNum; i++)
	{
		if (_players[i]!=nullptr && _players[i]->GetUserId() == userId)
		{
			return _playerNum==1 && _players[0]->GetUserId() == userId;
		}
	}
	return false;
}

void Room::ClearGame()
{
	//删除游戏
	if (_game != nullptr)
	{
		delete _game;
		_game = nullptr;
	}
}

bool Room::JoinRoom(User* player)
{
	if (_playerNum >= _playerMaxNum)
	{
		return false;
	}
	_playerNum++;
	int pos = FindEmptyPos();
	if (pos == -1) return false;

	_players[pos] = player;

	//下发房间内所有玩家信息
	BroadcastPlayerJoin(pos);

	SendRoomInfo(pos);

	return true;
}

int Room::FindEmptyPos()
{
	for (int i = 0; i < _playerMaxNum; i++)
	{
		if (_players[i] == nullptr)
		{
			return i;
		}
	}
	return -1;
}

void Room::BroadcastPlayerJoin(int pos)
{
	User* player = _players[pos];
	cJSON* root = cJSON_CreateObject();
	cJSON* cmd = cJSON_CreateString("SC_PlayerJoin");
	cJSON_AddItemToObject(root, "cmd", cmd);
	cJSON_AddItemToObject(root, "roomId", cJSON_CreateNumber(_roomId));
	cJSON_AddItemToObject(root, "pos", cJSON_CreateNumber(pos));
	cJSON_AddItemToObject(root, "userId", cJSON_CreateNumber(player->GetUserId()));
	cJSON_AddItemToObject(root, "nickname", cJSON_CreateString(player->GetNickname().c_str()));
	cJSON_AddItemToObject(root, "playerNum", cJSON_CreateNumber(_playerNum));

	for (int i = 0; i < _playerMaxNum; i++)
	{
		//发给房间内的用户
		if (_players[i] != nullptr && _players[i] != player)
		{
			std::cout << "To UserId:" << _players[i]->GetUserId() << "In RoomId:" << _roomId << std::endl;
			_players[i]->SendJSON(root);
		}
	}
	cJSON_Delete(root);
}

void Room::SendRoomInfo(int pos)
{
	cJSON* root = cJSON_CreateObject();
	cJSON* cmd = cJSON_CreateString("SC_RoomInfo");
	cJSON_AddItemToObject(root, "cmd", cmd);
	cJSON_AddItemToObject(root, "roomId", cJSON_CreateNumber(_roomId));
	cJSON_AddItemToObject(root, "roomName", cJSON_CreateString(_roomName.c_str()));
	cJSON_AddItemToObject(root, "roomOwnerId", cJSON_CreateNumber(_roomOwnerId));
	cJSON_AddItemToObject(root, "playerNum", cJSON_CreateNumber(_playerNum));
	cJSON_AddItemToObject(root, "playerMaxNum", cJSON_CreateNumber(_playerMaxNum));
	cJSON_AddItemToObject(root, "selfPos", cJSON_CreateNumber(pos));

	cJSON* players = cJSON_CreateArray();

	for (int i = 0; i < _playerMaxNum; i++)
	{
		if (_players[i] != nullptr)
		{
			cJSON* player = cJSON_CreateObject();
			cJSON_AddItemToObject(player, "pos", cJSON_CreateNumber(i));

			int playerUserId = _players[i]->GetUserId();
			std::string strPlayerNickname = _players[i]->GetNickname();
			int playerReadyState = _players[i]->GetIsReady() ? 1 : 0;

			cJSON_AddItemToObject(player, "userId", cJSON_CreateNumber(playerUserId));
			cJSON_AddItemToObject(player, "nickname", cJSON_CreateString(strPlayerNickname.c_str()));
			cJSON_AddItemToObject(player, "readyState", cJSON_CreateNumber(playerReadyState));
			cJSON_AddItemToArray(players, player);
		}
	}
	cJSON_AddItemToObject(root, "players", players);

	if (pos != -1)
	{
		//发给加入房间的用户
		std::cout << "To UserId:" << _players[pos]->GetUserId() << "In RoomId:" << _roomId << std::endl;
		_players[pos]->SendJSON(root);
	}
	cJSON_Delete(root);

}