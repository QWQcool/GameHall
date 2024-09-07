#include "GameClient.h"
//#include "RoomClient.h"
//#include "PlayerClient.h"
#include <unordered_map>

// 静态成员变量定义
GameClient* GameClient::instance = nullptr;

void GameClient::OnConnect()
{
	std::cout << "OnConnect successful!" << std::endl;
}

int GameClient::OnNetMsg(const char* buf, int len)
{
	if (len <= 2) return 0;
	uint16_t size = *(uint16_t*)buf;

	if (size > len) return 0;

	const char* data = buf + 2;
	std::string msg(data, size - 2);
	printf("Client recv<-----%s\n", msg.c_str());

	cJSON* root = cJSON_Parse(data);

	if (root == nullptr) return size;

	cJSON* cmd = cJSON_GetObjectItem(root, "cmd");

	if (strcmp("SC_Register", cmd->valuestring) == 0)
	{
		cJSON* msg = cJSON_GetObjectItem(root, "msg");
		if (strcmp(msg->valuestring, "Register Success") == 0)
		{
			std::cout << "Register successful!" << std::endl;
			SC_RegisterEvent(root);
		}
		else std::cout << "Client" << msg->valuestring<<std::endl;
	}
	else if (strcmp("SC_Login", cmd->valuestring) == 0)
	{
		cJSON* msg = cJSON_GetObjectItem(root, "msg");
		if (strcmp(msg->valuestring, "Login Success") == 0)
		{
			SC_LoginEvent(root);
		}
		else std::cout << "Login Failed" << std::endl;
	}
	else if (strcmp("SC_TokenLogin", cmd->valuestring) == 0)
	{
		cJSON* msg = cJSON_GetObjectItem(root, "msg");
		if (strcmp(msg->valuestring, "Token Login Success") == 0)
		{
			SC_TokenLoginEvent(root);
		}
		else std::cout <<"Client:" << msg->valuestring << std::endl;
	}
	else if (strcmp("SC_PreemptLogin", cmd->valuestring) == 0)
	{
		SC_PreemptLoginEvent(root);
	}
	else if (strcmp("SC_PreemptLoginSure", cmd->valuestring) == 0)
	{
		SC_SurePreemptLoginEvent(root);
	}
	else if (strcmp("SC_Logout", cmd->valuestring) == 0||strcmp("SC_ClientClose",cmd->valuestring)==0)
	{
		SC_LogoutEvent(root);
	}
	else if (strcmp("SC_CreateRoom", cmd->valuestring) == 0)
	{
		SC_CreateRoomEvent(root);
	}
	else if (strcmp("SC_RefreshRoomList", cmd->valuestring) == 0)
	{
		SC_RefreshRoomListEvent(root);
	}
	else if (strcmp("SC_JoinRoom", cmd->valuestring) == 0)
	{
		SC_JoinRoomEvent(root);
	}
	else if (strcmp("SC_ExitRoom",cmd->valuestring)==0)
	{
		SC_ExitRoomEvent(root);
	}
	else if (strcmp("SC_RoomInfo", cmd->valuestring) == 0)
	{
		SC_RoomInfoEvent(root);
	}
	else if (strcmp("SC_PlayerJoin", cmd->valuestring) == 0)
	{
		SC_PlayerJoinEvent(root);
	}
	else if (strcmp("SC_UpdatePos", cmd->valuestring) == 0 && _clientRoom!=nullptr)
	{
		SC_UpdatePosEvent(root);
	}
	else if (strcmp("SC_ReadyPos", cmd->valuestring) == 0 && _clientRoom != nullptr)
	{
		SC_UpdateReadyPosEvent(root);
	}
	else if (strcmp("SC_PlayGame", cmd->valuestring) == 0 && _clientRoom != nullptr)
	{
		SC_GoBangPlayEvent(root);
	}
	else if (strcmp("SC_GoBangDown", cmd->valuestring) == 0)
	{
		SC_GoBangDownEvent(root);
	}
	else if (strcmp("SC_AgainGame", cmd->valuestring) == 0)
	{
		SC_AgainGameEvent(root);
	}
	else if (strcmp("SC_ReturnRoom", cmd->valuestring) == 0)
	{
		SC_ReturnRoomEvent(root);
	}
	else {
		printf("未知命令 [%s]\n", cmd->valuestring);
	}

	cJSON_Delete(root);
	return size;
}

GameClient& GameClient::GetInstance()
{
	if (instance == nullptr) {
		instance = new GameClient();
	}
	return *instance;
}

void GameClient::SendMsg(const char* buf, int len)
{
	printf("Client send----->%s\n", buf);
	uint16_t size = len + 2;
	char* buff = new char[size];
	memset(buff, 0, sizeof(2));

	memcpy(buff, &size, 2);
	memcpy(buff + 2, buf, len);

	Send(buff, size);
	delete[] buff;
}

void GameClient::cJSONSendMsg(cJSON* root)
{
	char* data = cJSON_PrintUnformatted(root);
	int len = strlen(data);

	SendMsg(data, len);

	cJSON_Delete(root);
	free(data);
}

void GameClient::SendLoginRequest(const char* username, const char* password)
{
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "CS_Login");
	cJSON_AddStringToObject(root, "username", username);
	cJSON_AddStringToObject(root, "password", password);
	cJSONSendMsg(root);
}

void GameClient::SendTokenLoginRequest(const char* token)
{
	//废弃，日后可能使用
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "CS_TokenLogin");
	cJSON_AddStringToObject(root, "token", token);
	cJSONSendMsg(root);
}

void GameClient::SendSurePreemptLoginRequest()
{
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "CS_SurePreemptLogin");
	cJSON_AddStringToObject(root, "username", _username.c_str());
	cJSON_AddStringToObject(root, "password", _password.c_str());

	//cJSON_AddStringToObject(root, "token", _token.c_str());
	cJSONSendMsg(root);
}

void GameClient::SendRegisterRequest(const char* username,const char* password,const char* nickname)
{
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "CS_Register");
	cJSON_AddStringToObject(root, "nickname", nickname);
	cJSON_AddStringToObject(root, "username", username);
	cJSON_AddStringToObject(root, "password", password);

	cJSONSendMsg(root);
}

void GameClient::SendLogoutRequest()
{
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "CS_Logout");

	cJSON_AddNumberToObject(root, "userId", _clientPlayer->GetUserId());

	cJSONSendMsg(root);
}

void GameClient::SendCreateRoomRequest(const char* roomName, const char* gameType, int roomOwnerId)
{
	std::cout << "Client Send CreateRoomRequest" << std::endl;
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "CS_CreateRoom");
	cJSON_AddStringToObject(root, "roomName", roomName);
	cJSON_AddStringToObject(root, "roomGameType", gameType);
	cJSON_AddNumberToObject(root, "roomOwnerId", roomOwnerId);

	cJSONSendMsg(root);
}

void GameClient::SendRefreshRoomListRequest()
{
	std::cout << "Client Send RefreshRoomListRequest" << std::endl;
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "CS_RefreshRoomList");
	//userId
	cJSON_AddNumberToObject(root, "userId", _clientPlayer->GetUserId());

	cJSONSendMsg(root);
}

void GameClient::SendJoinRoomRequest(int roomId, int userId)
{
	std::cout << "Client Send JoinRoomRequest" << std::endl;
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "CS_JoinRoom");
	cJSON_AddNumberToObject(root, "roomId", roomId);
	cJSON_AddNumberToObject(root, "userId", userId);

	cJSONSendMsg(root);
}

void GameClient::SendExitRoomRequest(int roomId)
{
	std::cout << "Client Send ExitRoomRequest" << std::endl;
	cJSON* root = cJSON_CreateObject();

	int userId = _clientPlayer->GetUserId();

	cJSON_AddStringToObject(root, "cmd", "CS_ExitRoom");
	cJSON_AddNumberToObject(root, "roomId", roomId); //想退出的房间
	cJSON_AddNumberToObject(root, "userId", userId); //退出房间的人 ID标识

	cJSONSendMsg(root);
}

void GameClient::SendMovePosRequest(int roomId,int userId, int oldPos,int newPos)
{
	if (_clientRoom)
	{
		std::cout << "Client Send MovePosRequest" << std::endl;
		cJSON* root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "cmd", "CS_MovePos");
		cJSON_AddNumberToObject(root, "roomId", roomId);
		cJSON_AddNumberToObject(root, "userId", userId);
		cJSON_AddNumberToObject(root, "oldPos", oldPos);
		cJSON_AddNumberToObject(root, "newPos", newPos);

		cJSONSendMsg(root);
	}
	else
	{
		std::cout << "room is not exist" << std::endl;
	}
}

void GameClient::SendReadyPosRequest(int roomId, int userId, int pos, int readyState)
{
	if (_clientRoom)
	{
		std::cout << "Client Send ReadyPosRequest" << std::endl;
		cJSON* root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "cmd", "CS_ReadyPos");
		cJSON_AddNumberToObject(root, "roomId", roomId);
		cJSON_AddNumberToObject(root, "userId", userId);
		cJSON_AddNumberToObject(root, "pos", pos);
		cJSON_AddNumberToObject(root, "readyState", readyState);

		cJSONSendMsg(root);
	}
	else
	{
		std::cout << "room is not exist" << std::endl;
	}
}

void GameClient::SendGamePlayRequest(int roomId)
{
	if (_clientRoom)
	{
		std::cout << "Client Send GamePlayRequest" << std::endl;
		cJSON* root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "cmd", "CS_GamePlay");
		cJSON_AddNumberToObject(root, "roomId", roomId);
		cJSON_AddNumberToObject(root, "userId", _clientPlayer->GetUserId());
		cJSONSendMsg(root);
	}
	else
	{
		std::cout << "room is not exist" << std::endl;
	}
}

void GameClient::SendGoBangDownRequest(int x, int y, int playerIndex)
{
	if (_clientRoom->GetGameBase()!=nullptr)
	{
		std::cout << "Client Send GoBangDownRequest" << std::endl;
		cJSON* root = cJSON_CreateObject();
		int roomId = _clientRoom->GetRoomId();
		cJSON_AddStringToObject(root, "cmd", "CS_GoBangDown");
		cJSON_AddNumberToObject(root, "userId", _clientPlayer->GetUserId());
		cJSON_AddNumberToObject(root, "roomId", roomId);
		cJSON_AddNumberToObject(root, "xPos", x);
		cJSON_AddNumberToObject(root, "yPos", y);
		cJSON_AddNumberToObject(root, "playerIndex", playerIndex);
		cJSONSendMsg(root);
	}
	else
	{
		std::cout << "room is not exist" << std::endl;
	}
}

void GameClient::SendAgainGameRequest(int againState)
{
	if (_clientRoom->GetGameBase() != nullptr)
	{
		std::cout << "Client Send GoBangDownRequest" << std::endl;
		cJSON* root = cJSON_CreateObject();
		int userId = _clientPlayer->GetUserId();
		int roomId = _clientRoom->GetRoomId();
		std::string gameType = _clientRoom->GetRoomGameType();
		cJSON_AddStringToObject(root, "cmd", "CS_AgainGame");
		cJSON_AddNumberToObject(root, "roomId", roomId);
		cJSON_AddNumberToObject(root, "userId", userId);
		cJSON_AddStringToObject(root, "gameType", gameType.c_str());
		cJSON_AddNumberToObject(root, "againState", againState);
		cJSONSendMsg(root);
	}
	else
	{
		std::cout << "room is not exist" << std::endl;
	}
}

void GameClient::SendReturnRoomRequest(int Operator)
{
	if (_clientRoom->GetGameBase() != nullptr)
	{
		std::cout << "Client Send ReturnRoomRequest" << std::endl;
		cJSON* root = cJSON_CreateObject();
		int userId = _clientPlayer->GetUserId();
		int roomId = _clientRoom->GetRoomId();
		std::string gameType = _clientRoom->GetRoomGameType();
		cJSON_AddStringToObject(root, "cmd", "CS_ReturnRoom");
		cJSON_AddNumberToObject(root, "roomId", roomId);
		cJSON_AddNumberToObject(root, "userId", userId);
		cJSON_AddStringToObject(root, "gameType", gameType.c_str());
		cJSON_AddNumberToObject(root, "Operator", Operator);
		cJSONSendMsg(root);
	}
	else
	{
		std::cout << "room is not exist" << std::endl;
	}
}

void GameClient::SC_LoginEvent(cJSON* root)
{
	std::cout << "Client SC_LoginEvent Success" << std::endl;

	cJSON* player = cJSON_GetObjectItem(root, "player");
	cJSON* userId = cJSON_GetObjectItem(player, "playerId");
	cJSON* nickname = cJSON_GetObjectItem(player, "nickname");

	Player* clientPlayer = new Player(userId->valueint, nickname->valuestring);
	_clientPlayer = clientPlayer;
	this->_widgetType = Game_Hall;
	//同时获取最新的房间列表
	//SendRefreshRoomListRequest();
}

void GameClient::SC_TokenLoginEvent(cJSON* root)
{
	std::cout << "Client SC_TokenLoginEvent Success" << std::endl;
	cJSON* player = cJSON_GetObjectItem(root, "player");
	cJSON* userId = cJSON_GetObjectItem(player, "playerId");
	cJSON* nickname = cJSON_GetObjectItem(player, "nickname");
	//cJSON* token = cJSON_GetObjectItem(player, "token");
	//_token = token->valuestring;

	Player* clientPlayer = new Player(userId->valueint, nickname->valuestring);
	_clientPlayer = clientPlayer;
	this->_widgetType = Game_Hall;
	//同时获取最新的房间列表
	SendRefreshRoomListRequest();
}

void GameClient::SC_PreemptLoginEvent(cJSON* root)
{
	std::cout << "Client SC_PreemptLoginEvent Success" << std::endl;

	cJSON* username = cJSON_GetObjectItem(root, "username");
	cJSON* password = cJSON_GetObjectItem(root, "password");

	if (username->valuestring == "" || password->valuestring == "")
	{
		std::cout << "Client SC_PreemptLoginEvent Failed" << std::endl;
		return;
	}

	_username = username->valuestring;
	_password = password->valuestring;

	//cJSON* token = cJSON_GetObjectItem(root, "token");
	//
	//拿到旧用户的Token值方便重新登陆时检索数据
	//_token = token->valuestring;
	//切换Widget里的登陆UI页面至是否抢占登陆页面


	this->_widgetType = Preempt_Login;

}

void GameClient::SC_SurePreemptLoginEvent(cJSON* root)
{
	std::cout << "Client SC_SurePreemptLoginRespondEvent Success" << std::endl;
	//TODO 登陆成功，且根据服务端的反应来切换页面，并赋予相关的值
	//消息接受处理，拿到旧用户的信息和状态
	//cJSON* token = cJSON_GetObjectItem(root, "token");
	cJSON* userId = cJSON_GetObjectItem(root, "userId");
	cJSON* nickname = cJSON_GetObjectItem(root, "nickname");
	cJSON* isInRoom = cJSON_GetObjectItem(root, "isInRoom");
	cJSON* isInGame = cJSON_GetObjectItem(root, "isInGame");

	//_token = token->valuestring;
	
	int clientUserId = userId->valueint;
	std::string clientNickname = nickname->valuestring;
	int clientIsInRoom = isInRoom->valueint;
	int clientIsInGame = isInGame->valueint;

	Player* clientPlayer = new Player(clientUserId, clientNickname);
	_clientPlayer = clientPlayer;

	_widgetType = Game_Hall;

	if (clientIsInRoom == 1)
	{
		/*房间列表更新*/
			cJSON* roomsArray = cJSON_GetObjectItem(root, "rooms");

	if (roomsArray && roomsArray->type == cJSON_Array)
	{
		size_t i, arraySize = cJSON_GetArraySize(roomsArray);
		for (i = 0; i < arraySize; i++)
		{
			cJSON* room = cJSON_GetArrayItem(roomsArray, i);
			if (room && room->type == cJSON_Object)
			{
				int roomId = cJSON_GetObjectItem(room, "roomId")->valueint;
				int playerNum = cJSON_GetObjectItem(room, "playerNum")->valueint;
				int playerMaxNum = cJSON_GetObjectItem(room, "playerMaxNum")->valueint;
				const char* roomName = cJSON_GetObjectItem(room, "roomName")->valuestring;
				const char* roomGameType = cJSON_GetObjectItem(room, "roomGameType")->valuestring;
				int roomOwnerId = cJSON_GetObjectItem(room, "roomOwnerId")->valueint;
				// 处理房间信息
				std::cout << "Room ID: " << roomId << ", Room Name: " << roomName << std::endl;
				if (_clientRoomMgr.find(roomId) == _clientRoomMgr.end())
				{
				auto room = new Room(roomId, playerNum, playerMaxNum, roomName, roomGameType, roomOwnerId);
				_clientRoomMgr.insert(std::make_pair(room->GetRoomId(), room));
				}
				else
				{
					_clientRoomMgr[roomId]->ChangeRoomInfo(playerNum, roomName,roomOwnerId);
				}
			}
		}
		_isUpdateRoomList = true;
	}
		/*------------*/
		int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
		auto iter = _clientRoomMgr[roomId];
		if (iter == nullptr)
		{
			std::cout<<"roomId error"<<std::endl;
			return;
		}
		_clientRoom = iter;
		_widgetType = Game_Room;
		if (clientIsInGame == 1)
		{ 
			iter->SetIsGameShow(1);
			//TODO 登陆成功，且在房间中，且在游戏中，需要重新进入游戏

		}
	}

}

void GameClient::SC_RegisterEvent(cJSON* root)
{
	std::cout << "Client SC_RegisterEvent Success" << std::endl;
	//目前不知道注册成功能做啥,跟服务端返回收到注册成功？
}

void GameClient::SC_LogoutEvent(cJSON* root)
{
	std::cout << "Client Logout Success" << std::endl;
	this->_widgetType = Game_Login;
	delete _clientPlayer;
	_clientPlayer = nullptr;
	_clientRoomMgr.clear();

}

void GameClient::SC_CreateRoomEvent(cJSON* root)
{
	std::cout << "Client CreateRoom Success" << std::endl;
	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	int playNum = cJSON_GetObjectItem(root, "playerNumber")->valueint;
	int playMaxNum = cJSON_GetObjectItem(root, "playerMaxNum")->valueint;
	const char* roomName = cJSON_GetObjectItem(root, "roomName")->valuestring;
	const char* roomGameType = cJSON_GetObjectItem(root, "roomGameType")->valuestring;
	int roomOwnerId = cJSON_GetObjectItem(root, "roomOwnerId")->valueint;

	// 创建房间
	//Room(int _roomId, int _playerNum, int playerMaxNum, const std::string & roomName, const std::string & roomGameType, const std::string & roomOwner)
	auto room = new Room(roomId, playNum, playMaxNum, roomName, roomGameType, roomOwnerId);
	_clientRoomMgr.insert(std::make_pair(room->GetRoomId(), room));
	//创造房间同时加入房间
	SendJoinRoomRequest(roomId, _clientPlayer->GetUserId());
}

void GameClient::SC_RefreshRoomListEvent(cJSON* root)
{
	if (!root)
	{
		std::cerr << "Error parsing JSON data." << std::endl;
		return;
	}
	cJSON* roomsArray = cJSON_GetObjectItem(root, "rooms");

	if (roomsArray && roomsArray->type == cJSON_Array)
	{
		size_t i, arraySize = cJSON_GetArraySize(roomsArray);
		for (i = 0; i < arraySize; i++)
		{
			cJSON* room = cJSON_GetArrayItem(roomsArray, i);
			if (room && room->type == cJSON_Object)
			{
				int roomId = cJSON_GetObjectItem(room, "roomId")->valueint;
				int playerNum = cJSON_GetObjectItem(room, "playerNum")->valueint;
				int playerMaxNum = cJSON_GetObjectItem(room, "playerMaxNum")->valueint;
				const char* roomName = cJSON_GetObjectItem(room, "roomName")->valuestring;
				const char* roomGameType = cJSON_GetObjectItem(room, "roomGameType")->valuestring;
				int roomOwnerId = cJSON_GetObjectItem(room, "roomOwnerId")->valueint;
				// 处理房间信息
				std::cout << "Room ID: " << roomId << ", Room Name: " << roomName << std::endl;
				if (_clientRoomMgr.find(roomId) == _clientRoomMgr.end())
				{
				auto room = new Room(roomId, playerNum, playerMaxNum, roomName, roomGameType, roomOwnerId);
				_clientRoomMgr.insert(std::make_pair(room->GetRoomId(), room));
				}
				else
				{
					_clientRoomMgr[roomId]->ChangeRoomInfo(playerNum, roomName,roomOwnerId);
				}
			}
		}
		_isUpdateRoomList = true;
	}
}

void GameClient::SC_JoinRoomEvent(cJSON* root)
{
	std::cout << "Client JoinRoom Success" << std::endl;
	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	int userId = cJSON_GetObjectItem(root, "userId")->valueint;
	//TODO
	//场景切换
	//房间类存储用户相关事件，
	for (auto it = _clientRoomMgr.begin(); it != _clientRoomMgr.end(); it++)
	{
		//找到对应的房间，将客户端存储的房间成员变量绑定
		int saveRoomId = it->second->GetRoomId();
		if (saveRoomId == roomId)
		{
			this->_clientRoom = it->second;
			break;
		}
	}
	if (this->_clientRoom)
	{
	_widgetType = Game_Room;
	}
	else
	{
		std::cout << "Client bind room error" << std::endl;
	}

}

void GameClient::SC_RoomInfoEvent(cJSON* root)
{
	std::cout << "RoomFlush Success" << std::endl;
	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	std::string roomName(cJSON_GetObjectItem(root, "roomName")->valuestring);
	int roomOwnerId = cJSON_GetObjectItem(root, "roomOwnerId")->valueint;
	int playerNum = cJSON_GetObjectItem(root, "playerNum")->valueint;
	int playerMaxNum = cJSON_GetObjectItem(root, "playerMaxNum")->valueint;

	cJSON* playerArray = cJSON_GetObjectItem(root, "players");

	if (_clientRoomMgr.find(roomId) != _clientRoomMgr.end())
	{
	auto iter = _clientRoomMgr[roomId];
	if (playerArray && playerArray->type == cJSON_Array)
	{
		size_t i, arraySize = cJSON_GetArraySize(playerArray);
		for (i = 0; i < arraySize; i++)
		{
			cJSON* player = cJSON_GetArrayItem(playerArray, i);
			if (player && player->type == cJSON_Object)
			{
				int pos = cJSON_GetObjectItem(player, "pos")->valueint;
				int userId = cJSON_GetObjectItem(player, "userId")->valueint;
				std::string nickname = cJSON_GetObjectItem(player, "nickname")->valuestring;
				int readyState = cJSON_GetObjectItem(player, "readyState")->valueint;
				// 处理房间内用户信息
				std::cout << "User ID: " << userId << " pos: " << pos << std::endl;
				if (userId == _clientPlayer->GetUserId())
				{
					//服务端分配给客户端的pos
					_clientPlayer->SetPosition(pos);
					iter->JoinRoom(_clientPlayer);
				}
				else
				{
					Player* newPlayer= new Player(userId, pos, nickname, readyState);
					iter->JoinRoom(newPlayer);
				}
			}
		}
	}
	//强制统一数据
	_clientRoom->SetPlayerNum(playerNum);
	}
	else
	{
		std::cout << "RoomId error cause SC_RoomInfoEvent Error"<<std::endl;
	}

}

void GameClient::SC_PlayerJoinEvent(cJSON* root)
{
	std::cout << "SC_PlayerJoin Success" << std::endl;
	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	int userId = cJSON_GetObjectItem(root, "userId")->valueint;
	std::string nickname = cJSON_GetObjectItem(root, "nickname")->valuestring;
	int pos = cJSON_GetObjectItem(root, "pos")->valueint;
	int playerNum = cJSON_GetObjectItem(root, "playerNum")->valueint;

	if (_clientRoomMgr.find(roomId) != _clientRoomMgr.end())
	{
		auto iter = _clientRoomMgr[roomId];
		Player* newPlayer = new Player(userId, pos, nickname);
		iter->JoinRoom(newPlayer);
	}
	//统一数据
	if (_clientRoom)
	{
		_clientRoom->SetPlayerNum(playerNum);
	}
	else
	{
		std::cout << "Client Room Error in PlayerJoinEvent" << std::endl;
	}
}

void GameClient::SC_ExitRoomEvent(cJSON* root)
{
	std::cout << "SC_ExieRoom Success" << std::endl;
	int exitRoomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	int exitUserId = cJSON_GetObjectItem(root, "userId")->valueint;
	int playerNum = cJSON_GetObjectItem(root, "playerNum")->valueint;
	int isDeleteRoom = cJSON_GetObjectItem(root, "isDeleteRoom")->valueint;
	int roomOwnerId = cJSON_GetObjectItem(root, "roomOwnerId")->valueint;

	_clientPlayer->SetUnReady();

	if (_clientRoomMgr.find(exitRoomId) != _clientRoomMgr.end())
	{
		Room* iter = _clientRoomMgr[exitRoomId];
		iter->ExitRoom(exitUserId);
		iter->SetPlayerNum(playerNum);
		if (isDeleteRoom)
		{
			_clientRoomMgr.erase(exitRoomId);
			delete iter;
		}
	}
	if (_clientPlayer->GetUserId() == exitUserId)
	{
		_widgetType = Game_Hall;
	}
	SendRefreshRoomListRequest();
}

void GameClient::SC_UpdatePosEvent(cJSON* root)
{
	std::cout << "SC_UpdatePosEvent Success" << std::endl;
	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	int userId = cJSON_GetObjectItem(root, "userId")->valueint;
	int oldPos = cJSON_GetObjectItem(root, "oldPos")->valueint;
	int newPos = cJSON_GetObjectItem(root, "newPos")->valueint;

	// Check if the client has the room
	auto roomIt = _clientRoomMgr.find(roomId);
	if (roomIt != _clientRoomMgr.end())
	{
		Room* room = roomIt->second;
		Player** players = room->GetPlayers();
		// Check if the room contains the player at the old position
		if (oldPos >= 0 && oldPos < room->GetPlayerMaxNum() &&
			newPos >= 0 && newPos < room->GetPlayerMaxNum() &&
			players[oldPos] && players[oldPos]->GetUserId() == userId)
		{
			// Move the player in the room's player array
			Player* tempPlayer = players[oldPos];
			players[oldPos] = nullptr;
			players[newPos] = tempPlayer;

			// Update the player's internal state (assuming the player stores its position)
			if (tempPlayer)
			{
				tempPlayer->SetPosition(newPos);  // Assuming SetPosition is a method of Player class
			}
		}
		else
		{
			std::cout << "Player not found or invalid positions." << std::endl;
		}
	}
	else
	{
		std::cout << "RoomId error cause SC_UpdatePosEvent Error" << std::endl;
	}
}

void GameClient::SC_UpdateReadyPosEvent(cJSON* root)
{
	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	int userId = cJSON_GetObjectItem(root, "userId")->valueint;
	int pos = cJSON_GetObjectItem(root, "pos")->valueint;
	int readyState = cJSON_GetObjectItem(root, "readyState")->valueint;
	std::cout <<roomId << ":SC_UpdateReadyPosEvent Success" << std::endl;
	// Check if the client has the room
	auto roomIt = _clientRoomMgr.find(roomId);
	if (roomIt != _clientRoomMgr.end())
	{
		Room* room = roomIt->second;
		Player** players = room->GetPlayers();
		// Check if the room contains the player at the old position
		if (pos >= 0 && pos < room->GetPlayerMaxNum())
		{
			if (players[pos] != nullptr && readyState==1)
			{
				players[pos]->SetReady();
			}
			else if (players[pos] != nullptr && readyState == 0)
			{
				players[pos]->SetUnReady();
			}
		}
		else
		{
			std::cout << "Player not found or invalid positions." << std::endl;
		}
	}
	else
	{
		std::cout << "RoomId error cause SC_UpdateReadyPosEvent Error" << std::endl;
	}
}

void GameClient::SC_GoBangPlayEvent(cJSON* root)
{
	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	int roomState = cJSON_GetObjectItem(root, "roomState")->valueint;
	std::cout << roomId << ":SC_GoBangPlayEvent Success" << std::endl;
	// Check if the client has the room
	auto roomIt = _clientRoomMgr.find(roomId);
	if (roomIt != _clientRoomMgr.end())
	{
		Room* room = roomIt->second;
		//房间开始游戏，房间切换为游戏界面状态
		room->GameStart();

		room->SetIsGameShow(roomState);//游戏界面show 要放在开始游戏后面
	}
	else
	{
		std::cout << "RoomId error cause SC_GoBangPlayEvent Error" << std::endl;
	}
}

void GameClient::SC_GoBangDownEvent(cJSON* root)
{
	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	int xPos = cJSON_GetObjectItem(root, "xPos")->valueint;
	int yPos = cJSON_GetObjectItem(root, "yPos")->valueint;
	int playerIndex = cJSON_GetObjectItem(root, "playerIndex")->valueint;
	int nextPlayerIndex = cJSON_GetObjectItem(root, "nextPlayerIndex")->valueint;
	int GameWiner = cJSON_GetObjectItem(root, "GameWiner")->valueint;
	int whiteScore = cJSON_GetObjectItem(root, "whiteScore")->valueint;
	int blackScore = cJSON_GetObjectItem(root, "blackScore")->valueint;

	std::cout << roomId << ":SC_GoBangDownEvent Success" << std::endl;


	if (_clientRoom != nullptr)
	{
		GameBase* game = _clientRoom->GetGameBase();

		if (game != nullptr && dynamic_cast<GoBang*>(game))
		{
			// 这是一个 GoBang 实例
			static_cast<GoBang*>(game)->DownPlayer(xPos, yPos, playerIndex,nextPlayerIndex);
			std::cout << xPos << "|" << yPos << "|Down|" << playerIndex << std::endl;
			//TODO 如果GameState!=0 决出胜负 ，游戏结算画面
			static_cast<GoBang*>(game)->ResetTimerEndDown();
			if (GameWiner != 0)
			{
				static_cast<GoBang*>(game)->SetScore(blackScore, whiteScore);
				static_cast<GoBang*>(game)->SetGameState(2); //GameState为2时切换为游戏结束页面
				//TODO一些游戏结束的逻辑，也可以写在游戏类里
			}
		}
	}
	else
	{
		std::cout << "RoomId error cause SC_GoBangDownEvent Error" << std::endl;
	}
}

void GameClient::SC_AgainGameEvent(cJSON* root)
{
	int userId = cJSON_GetObjectItem(root, "userId")->valueint;
	int roomId = cJSON_GetObjectItem(root, "roomID")->valueint;
	std::string gameType = cJSON_GetObjectItem(root, "gameType")->valuestring;
	int againState = cJSON_GetObjectItem(root, "againState")->valueint;

	GameBase* game = _clientRoom->GetGameBase();
	if (game != nullptr && dynamic_cast<GoBang*>(game) && gameType==u8"五子棋")
	{
		static_cast<GoBang*>(game)->SetContinueGameState(againState);
		if (againState == 2)
		{
			//切换回游戏开始状态
			static_cast<GoBang*>(game)->SetGameState(1);
			//重置游戏状态
			static_cast<GoBang*>(game)->Init();
		}
	}

}

void GameClient::SC_ReturnRoomEvent(cJSON* root)
{
	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	int userId = cJSON_GetObjectItem(root, "userId")->valueint;
	int Operator = cJSON_GetObjectItem(root, "Operator")->valueint;
	std::string gameType = cJSON_GetObjectItem(root, "gameType")->valuestring;
	std::cout << roomId << ":SC_ReturnRoomEvent Success" << std::endl;
	if (_clientRoom != nullptr && _clientRoom->GetRoomId()==roomId)
	{
			_clientRoom->SetIsGameShow(0);
			_clientRoom->ClearGame();
	}
}

void GameClient::SC_DeleteRoomEvent(cJSON* root)
{
	int userId = cJSON_GetObjectItem(root, "userId")->valueint;
	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;

	if (_clientRoomMgr.find(roomId) != _clientRoomMgr.end())
	{
		Room* iter = _clientRoomMgr[roomId];
		_clientRoomMgr.erase(roomId);
		delete iter;
	}
	_isUpdateRoomList = true;
}
