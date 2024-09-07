#include "../Session.h"
#include "../GameServer.h"
#include "../User.h"
#include "../RoomMgr.h"
#include "../Room.h"
#include "../GameBase.hpp"

class GamePlay_GameCell : public IGameCell
{
public:
	uint64_t netId;
	int userId;
	int roomId;

public:
	virtual void Exec() override
	{
		Session* session = GameServer::Ins()->FindNetIdSession(netId);
		if (nullptr == session) {
			LOG_ERROR << "û���ҵ���Ӧ�� Session:" << netId;
			return;
		}
		User* user = GameServer::Ins()->FindUser(userId);
		if (nullptr == user) {
			LOG_ERROR << "û���ҵ���Ӧ�� User:" << session->GetNetId();
			return;
		}
		Room* room = user->GetRoom();
		if (nullptr == room)
		{
			LOG_ERROR << "û���ҵ���Ӧ�� Room:" << room;
			return;
		}
		room->SC_GamePlay(userId, roomId);

	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

void Room::CS_GamePlay(cJSON* root)
{
	const char* msg = cJSON_Print(root);
	LOG_INFO << "�յ�cJSON:\n" << msg;
	delete msg;

	int userId = cJSON_GetObjectItem(root, "userId")->valueint;
	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	int roomState = 1;

	GamePlay_GameCell* cell = new GamePlay_GameCell;
	cell->netId = GetNetId(userId);
	cell->userId = userId;
	cell->roomId = roomId;

}

void Room::SC_GamePlay(int userId, int roomId)
{

	int roomState = 1;
	cJSON* msg = cJSON_CreateObject();
	cJSON_AddStringToObject(msg, "cmd", "SC_GamePlay");
	cJSON_AddNumberToObject(msg, "roomId", roomId);
	cJSON_AddNumberToObject(msg, "roomState", roomState);
	cJSON_AddNumberToObject(msg, "result", 1);

	//�㲥�������������û�
	for (int i = 0; i < _playerMaxNum; i++)
	{
		if (_players[i] != nullptr)
		{
			_players[i]->SendJSON(msg);
		}
	}

	if (_game == nullptr)
	{
		_game = GameFactory::CreateGame(_roomGameType);
	}

	cJSON_Delete(msg);
}

