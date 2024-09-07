#include "../Session.h"
#include "../GameServer.h"
#include "../User.h"
#include "../RoomMgr.h"
#include "../Room.h"
#include "../GameBase.hpp"

class AgainGame_GameCell : public IGameCell
{
public:
	uint64_t netId;
	int userId;
	int roomId;
	std::string gameType;
	int againState;
public:
	virtual void Exec() override
	{
		Session* session = GameServer::Ins()->FindNetIdSession(netId);
		if (nullptr == session) {
			LOG_ERROR << "没有找到对应的 Session:" << netId;
			return;
		}
		User* user = GameServer::Ins()->FindUser(userId);
		if (nullptr == user) {
			LOG_ERROR << "没有找到对应的 User:" << session->GetNetId();
			return;
		}
		Room* room = user->GetRoom();
		if (nullptr == room)
		{
			LOG_ERROR << "没有找到对应的 Room:" << room;
			return;
		}
		GameBase* game = room->GetGameBase();
		if (nullptr == game)
		{
			LOG_ERROR << "没有找到对应的 Game:" << room;
			return;
		}

		room->SC_AgainGame(userId, roomId,gameType,againState);
	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

void Room::CS_AgainGame(cJSON* root)
{
	const char* msg = cJSON_Print(root);
	LOG_INFO << "收到cJSON:\n" << msg;
	delete msg;

	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	int userId = cJSON_GetObjectItem(root, "userId")->valueint;
	std::string gameType = cJSON_GetObjectItem(root, "gameType")->valuestring;
	int againState = cJSON_GetObjectItem(root, "againState")->valueint;
	againState++;

	AgainGame_GameCell * cell = new AgainGame_GameCell();
	cell->netId = GetNetId(userId);
	cell->userId = userId;
	cell->roomId = roomId;
	cell->gameType = gameType;
	cell->againState = againState;
	GameServer::Ins()->Post(cell);
}

void Room::SC_AgainGame(int userId, int roomId, const std::string& gameType,int againState)
{
	if (gameType == u8"五子棋" && _game != nullptr && dynamic_cast<GoBang*>(_game))
	{
		static_cast<GoBang*>(_game)->SetContinueGameState(againState);
		if (againState == 2)
		{
			//初始化棋盘和继续游戏状态
			static_cast<GoBang*>(_game)->SetContinueGameState(0);
			static_cast<GoBang*>(_game)->Init();
		}
	}
	else
	{
		LOG_ERROR << "没有找到对应的游戏类型:" << gameType;
		return;
	}

	//处理完后发送回去
	cJSON* msg = cJSON_CreateObject();
	cJSON_AddStringToObject(msg, "cmd", "SC_AgainGame");
	cJSON_AddNumberToObject(msg, "roomId", roomId);
	cJSON_AddNumberToObject(msg, "userId", userId);
	cJSON_AddStringToObject(msg, "gameType", gameType.c_str());
	cJSON_AddNumberToObject(msg, "againState", againState);
	cJSON_AddNumberToObject(msg, "result", 1);

	for (int i = 0; i < _playerMaxNum; i++)
	{
		if (_players[i] != nullptr)
		{
			_players[i]->SendJSON(msg);
		}
	}
	cJSON_Delete(msg);
}

