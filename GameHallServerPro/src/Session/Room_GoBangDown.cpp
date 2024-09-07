#include "../Session.h"
#include "../GameServer.h"
#include "../User.h"
#include "../RoomMgr.h"
#include "../Room.h"
#include "../GameBase.hpp"

class GoBangDown_GameCell : public IGameCell
{
public:
	uint64_t netId;
	int userId;
	int roomId;
	int xPos;
	int yPos;
	int playerIndex;

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
		room->SC_GoBangDown(userId, roomId, xPos, yPos, playerIndex);
	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

void Room::CS_GoBangDown(cJSON* root)
{
	const char* msg = cJSON_Print(root);
	LOG_INFO << "收到cJSON:\n" << msg;
	delete msg;

	int userId = cJSON_GetObjectItem(root, "userId")->valueint;
	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	int xPos = cJSON_GetObjectItem(root, "xPos")->valueint;
	int yPos = cJSON_GetObjectItem(root, "yPos")->valueint;
	int playerIndex = cJSON_GetObjectItem(root, "playerIndex")->valueint;

	GoBangDown_GameCell* cell = new GoBangDown_GameCell();
}

void Room::SC_GoBangDown(int userId, int roomId, int xPos, int yPos, int playerIndex)
{
	int nextPlayerIndex = 0;
	int GameWiner = 0;

	int blackScore = 0;
	int whiteScore = 0;

	if (_game != nullptr && dynamic_cast<GoBang*>(_game))
	{
		if (playerIndex != static_cast<GoBang*>(_game)->GetPlayerIndex())
		{
			std::cout << "In Send_SC_GoBangDown Error: Player index does not match" << std::endl;
			return;
		}
		static_cast<GoBang*>(_game)->SetDown(xPos, yPos, playerIndex);//SetDown会改变游戏类里的PlayerIndex
		nextPlayerIndex = static_cast<GoBang*>(_game)->GetPlayerIndex();
		bool gameWinerBool = static_cast<GoBang*>(_game)->CheckForWin(xPos, yPos, playerIndex);
		if (gameWinerBool)
		{
			GameWiner = playerIndex;
		}
	}

	blackScore = static_cast<GoBang*>(_game)->GetBlackScore();
	whiteScore = static_cast<GoBang*>(_game)->GetWhiteScore();


	cJSON* msg = cJSON_CreateObject();
	cJSON_AddStringToObject(msg, "cmd", "SC_GoBangDown");
	cJSON_AddNumberToObject(msg, "roomId", roomId);
	cJSON_AddNumberToObject(msg, "xPos", xPos);
	cJSON_AddNumberToObject(msg, "yPos", yPos);
	cJSON_AddNumberToObject(msg, "blackScore", blackScore);
	cJSON_AddNumberToObject(msg, "whiteScore", whiteScore);
	cJSON_AddNumberToObject(msg, "playerIndex", playerIndex);
	cJSON_AddNumberToObject(msg, "nextPlayerIndex", nextPlayerIndex);
	cJSON_AddNumberToObject(msg, "GameWiner", GameWiner);
	cJSON_AddNumberToObject(msg, "result", 1);

	//广播给房间内所有用户
	for (int i = 0; i < _playerMaxNum; i++)
	{
		if (_players[i] != nullptr)
		{
			_players[i]->SendJSON(msg);
		}
	}

	cJSON_Delete(msg);
}

