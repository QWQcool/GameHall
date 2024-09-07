#include "../Session.h"
#include "../GameServer.h"
#include "../User.h"
#include "../RoomMgr.h"
#include "../Room.h"

class ReadyPos_GameCell : public IGameCell
{
public:
	uint64_t netId;
	int userId;
	int roomId;
	int pos;
	int readyState;
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

		room->SC_ReadyPos(userId,roomId,pos,readyState);
	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

void Room::CS_ReadyPos(cJSON* root)
{
	const char* msg = cJSON_Print(root);
	LOG_DEBUG << "收到cJSON:\n" << msg;
	delete msg;

	int roomId = cJSON_GetObjectItem(root, "roomId")->valueint;
	int userId = cJSON_GetObjectItem(root, "userId")->valueint;
	int pos = cJSON_GetObjectItem(root, "pos")->valueint;
	int readyState = cJSON_GetObjectItem(root, "readyState")->valueint;

	//state原本等于0 说明之前没准备，原本等于1说明之前准备了
	if (readyState == 0)
	{
		readyState = 1;
	}
	else
	{
		readyState = 0;
	}
	ReadyPos_GameCell* cell = new ReadyPos_GameCell();
	cell->netId = GetNetId(userId);
	cell->userId = userId;
	cell->roomId = roomId;
	cell->pos = pos;
	cell->readyState = readyState;
	GameServer::Ins()->Post(cell);
}

void Room::SC_ReadyPos(int userId, int roomId, int pos, int readyState)
{
	//处理完后发送回去
	cJSON* msg = cJSON_CreateObject();
	cJSON_AddStringToObject(msg, "cmd", "SC_ReadyPos");
	cJSON_AddNumberToObject(msg, "roomId", roomId);
	cJSON_AddNumberToObject(msg, "userId", userId);
	cJSON_AddNumberToObject(msg, "pos", pos);
	cJSON_AddNumberToObject(msg, "readyState", readyState);

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

