#include "../Session.h"
#include "../GameServer.h"
#include "../User.h"
#include "../RoomMgr.h"
#include "../Room.h"
#include "../GameBase.hpp"

class DeleteRoom_GameCell : public IGameCell
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
			LOG_ERROR << "没有找到对应的 Session:" << netId;
			return;
		}
		User* user = GameServer::Ins()->FindUser(userId);
		if (nullptr == user) {
			LOG_ERROR << "没有找到对应的 User:" << session->GetNetId();
			return;
		}
		user->SC_DeleteRoom(userId, roomId);

	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

void User::ServerDeleteRoom(int userId, int roomId)
{
	DeleteRoom_GameCell* cell = new DeleteRoom_GameCell();
	cell->netId = GetNetId();
	cell->userId = userId;
	cell->roomId = roomId;

	LOG_INFO << "User::ServerDeleteRoom:" << cell->roomId;

	GameServer::Ins()->Post(cell);
}

void User::CS_DeleteRoom(cJSON* root)
{
	const char* msg = cJSON_Print(root);
	LOG_INFO << "收到cJSON:\n" << msg;
	delete msg;

	cJSON* userIdJson = cJSON_GetObjectItem(root, "userId");
	int userId = userIdJson->valueint;
	cJSON* roomIdJson = cJSON_GetObjectItem(root, "roomId");
	int roomId = roomIdJson->valueint;

	DeleteRoom_GameCell* cell = new DeleteRoom_GameCell();
	cell->netId = GetNetId();
	cell->userId = userId;
	cell->roomId = roomId;

	GameServer::Ins()->Post(cell);
}

void User::SC_DeleteRoom(int userId, int roomId)
{
	cJSON* SC_DeleteRoom = cJSON_CreateObject();
	cJSON_AddStringToObject(SC_DeleteRoom, "cmd", "SC_DeleteRoom");
	cJSON_AddNumberToObject(SC_DeleteRoom,"userId", userId);
	cJSON_AddNumberToObject(SC_DeleteRoom,"roomId", roomId);

	//避免重复删除
	Room* room = RoomMgr::GetInstance().FindRoom(roomId);

	if (room != nullptr)
	{
		RoomMgr::GetInstance().DeleteRoom(roomId);
	}

	SendJSON(SC_DeleteRoom);
	cJSON_Delete(SC_DeleteRoom);
}

