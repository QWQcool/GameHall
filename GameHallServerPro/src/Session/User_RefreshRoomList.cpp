#include "../Session.h"
#include "../GameServer.h"
#include "../User.h"
#include "../Room.h"
#include "../RoomMgr.h"

class RefreshRoomList_GameCell : public IGameCell
{
public:
	uint64_t netId;
	int userId;

public:
	virtual void Exec() override
	{
		Session* session = GameServer::Ins()->FindNetIdSession(netId);
		if (nullptr == session)
		{
			LOG_ERROR << "没有找到对应的 Session: " << netId;
			return;
		}
		User* user = GameServer::Ins()->FindUser(userId);
		if (nullptr == user)
		{
			LOG_ERROR << "没有找到对应的 User: " << userId;
			return;
		}

		user->SC_RefreshRoomList();
	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

void User::CS_RefreshRoomList(cJSON* root)
{
	const char* msg = cJSON_Print(root);
	LOG_INFO << "收到cJSON:\n" << msg;
	delete msg;

	cJSON* userIdJson = cJSON_GetObjectItem(root, "userId");

	if (nullptr == userIdJson || userIdJson->type != cJSON_Number)
	{
		LOG_ERROR << "CS_RefreshRoomList: 用户ID为空||类型错误";
		return;
	}


	RefreshRoomList_GameCell* cell = new RefreshRoomList_GameCell();
	cell->netId = this->GetNetId();
	cell->userId = userIdJson->valueint;

	GameServer::Ins()->Post(cell);
}

void User::SC_RefreshRoomList()
{
	cJSON* SC_RefreshRoomList = cJSON_CreateObject();

	cJSON_AddStringToObject(SC_RefreshRoomList, "cmd", "SC_RefreshRoomList");

	std::map<int, Room*> roomMap = RoomMgr::GetInstance().GetRoomListMap();
	cJSON* roomsArray = cJSON_CreateArray(); // 创建房间数组

	for (auto it = roomMap.begin(); it != roomMap.end(); it++)
	{
		cJSON* room = cJSON_CreateObject();
		int roomId = it->second->GetRoomId();
		int playerNum = it->second->GetPlayerNum();
		int playerMaxNum = it->second->GetPlayerMaxNum();
		std::string roomName = it->second->GetRoomName();
		std::string roomGameType = it->second->GetRoomGameType();
		int roomOwnerId = it->second->GetRoomOwner();

		cJSON_AddNumberToObject(room, "roomId", roomId);
		cJSON_AddNumberToObject(room, "playerNum", playerNum);
		cJSON_AddNumberToObject(room, "playerMaxNum", playerMaxNum);
		cJSON_AddStringToObject(room, "roomName", roomName.c_str());
		cJSON_AddStringToObject(room, "roomGameType", roomGameType.c_str());
		cJSON_AddNumberToObject(room, "roomOwnerId", roomOwnerId);

		cJSON_AddItemToArray(roomsArray, room); // 将房间对象添加到数组
	}

	cJSON_AddItemToObject(SC_RefreshRoomList, "rooms", roomsArray); // 将房间数组添加到主对象

	cJSON_AddNumberToObject(SC_RefreshRoomList, "result", 1);
	cJSON_AddStringToObject(SC_RefreshRoomList, "msg", "刷新房间列表成功");

	SendJSON(SC_RefreshRoomList);
	cJSON_Delete(SC_RefreshRoomList);
}