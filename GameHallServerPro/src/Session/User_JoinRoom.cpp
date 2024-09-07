#include "../Session.h"
#include "../GameServer.h"
#include "../User.h"
#include "../RoomMgr.h"

class JoinRoom_GameCell : public IGameCell
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

		user->SC_JoinRoom(userId, roomId);
	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

void User::CS_JoinRoom(cJSON* root)
{
	const char* msg = cJSON_Print(root);
	LOG_INFO << "收到cJSON:\n" << msg;
	delete msg;

	cJSON* roomId = cJSON_GetObjectItem(root, "roomId");
	cJSON* userId = cJSON_GetObjectItem(root, "userId");

	if (nullptr == roomId || nullptr == userId)
	{
		LOG_ERROR << "JoinRoom 参数错误";
		return;
	}

	//类型错误
	if (cJSON_Number != roomId->type || cJSON_Number != userId->type)
	{
		LOG_ERROR << "JoinRoom 类型错误";
		return;
	}

	JoinRoom_GameCell* cell = new JoinRoom_GameCell();
	cell->netId = this->GetNetId();
	cell->userId = userId->valueint;
	cell->roomId = roomId->valueint;

	GameServer::Ins()->Post(cell);

}

void User::SC_JoinRoom(int userId,int roomId)
{
	cJSON* sc_joinRoom = cJSON_CreateObject();
	cJSON_AddStringToObject(sc_joinRoom, "cmd", "SC_JoinRoom");
	cJSON_AddNumberToObject(sc_joinRoom, "roomId", roomId);
	cJSON_AddNumberToObject(sc_joinRoom, "userId", userId);
	cJSON_AddNumberToObject(sc_joinRoom, "result", 1);
	cJSON_AddStringToObject(sc_joinRoom, "msg", "JoinRoom Success");

	this->_room = RoomMgr::GetInstance().FindRoom(roomId);

	SendJSON(sc_joinRoom);

	RoomMgr::GetInstance().JoinRoomEvent(roomId, this);

	cJSON_Delete(sc_joinRoom);
}