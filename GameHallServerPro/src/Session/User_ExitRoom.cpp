#include "../Session.h"
#include "../GameServer.h"
#include "../User.h"
#include "../RoomMgr.h"
#include "../Room.h"

class ExitRoom_GameCell : public IGameCell
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
		if (nullptr == user)
		{
			LOG_ERROR << "没有找到对应的 User:" << userId;
			return;
		}

		user->SC_ExitRoom(userId, roomId);
	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

void User::CS_ExitRoom(cJSON* root)
{
	const char* msg = cJSON_Print(root);
	LOG_INFO << "收到cJSON:\n" << msg;
	delete msg;

	cJSON* userIdJson = cJSON_GetObjectItem(root, "userId");
	cJSON* roomIdJson = cJSON_GetObjectItem(root, "roomId");

	if (nullptr == userIdJson || nullptr == roomIdJson)
	{
		LOG_DEBUG << u8"userId or roomId is null";
		return;
	}

	if (userIdJson->valueint != this->_userInfo.uid)
	{
		LOG_DEBUG << u8"userId 不匹配";
		return;
	}

	//TODO 检查房间
	Room* room = RoomMgr::GetInstance().FindRoom(roomIdJson->valueint);
	if (nullptr == room)
	{
		LOG_DEBUG << u8"没有找到对应的 Room:" << roomIdJson->valueint;
		return;
	}

	if (_room == nullptr)
	{
		LOG_DEBUG << u8"没有加入房间";
		return;
	}

	ExitRoom_GameCell* cell = new ExitRoom_GameCell();
	cell->netId = this->GetNetId();
	cell->userId = userIdJson->valueint;
	cell->roomId = roomIdJson->valueint;

	GameServer::Ins()->Post(cell);

}

void User::SC_ExitRoom(int userId, int roomId)
{
	_isReady = false;

	int playerMaxNum = _room->GetPlayerMaxNum();
	int playerNum = _room->GetPlayerNum();
	int isDeleteRoom = 1;
	User** players = _room->GetPlayers();

	//服务端房间处理
	for (int i = 0; i < playerMaxNum; i++)
	{
		if (players[i] != nullptr && players[i]->GetUserId() == userId)
		{
			players[i] = nullptr;
			_room->RemovePlayerNum();
			break;
		}
	}

	//房主更替
	for (int i = 0; i < playerMaxNum; i++)
	{
		if (players[i] != nullptr)
		{
			int nextOwnerId = players[i]->GetUserId();
			_room->SetRoomOwner(nextOwnerId);
			isDeleteRoom = 0;
			break;
		}
	}
	cJSON* msg = cJSON_CreateObject();

	int roomOwnerId = _room->GetRoomOwner();
	int roomPlayerNum = _room->GetPlayerNum();
	cJSON_AddStringToObject(msg, "cmd", "SC_ExitRoom");
	cJSON_AddNumberToObject(msg, "roomId", roomId);
	cJSON_AddNumberToObject(msg, "userId", userId);
	cJSON_AddNumberToObject(msg, "roomOwnerId", roomOwnerId);
	cJSON_AddNumberToObject(msg, "playerNum", roomPlayerNum);
	cJSON_AddNumberToObject(msg, "isDeleteRoom", isDeleteRoom);
	cJSON_AddStringToObject(msg, "msg", "ExitRoom Success");
	cJSON_AddNumberToObject(msg, "result", 1);

	// 广播给房间内所有剩余用户
	for (int i = 0; i < playerMaxNum; i++)
	{
		if (players[i] != nullptr)
		{
			players[i]->SendJSON(msg);
		}
	}

	//因为先把自己移除了，所以需要发给自己
	SendJSON(msg);
	cJSON_Delete(msg);

	//如果房间没人删除房间
	if (isDeleteRoom)
	{
		RoomMgr::GetInstance().DeleteRoom(roomId);
	}
}