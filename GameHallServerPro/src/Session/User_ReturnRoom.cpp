#include "../Session.h"
#include "../GameServer.h"
#include "../User.h"
#include "../RoomMgr.h"
#include "../Room.h"

class ReturnRoom_GameCell : public IGameCell
{
public:
	uint64_t netId;
	int userId;
	int roomId;
	int Operator;
	std::string gameType;

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
		user->SC_ReturnRoom( userId,  roomId,  Operator, gameType);
	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

void User::CS_ReturnRoom(cJSON* root)
{
	const char* msg = cJSON_Print(root);
	LOG_INFO << "收到cJSON:\n" << msg;
	delete msg;

	cJSON* userId = cJSON_GetObjectItem(root, "userId");
	cJSON* roomId = cJSON_GetObjectItem(root, "roomId");
	cJSON* gameType = cJSON_GetObjectItem(root, "gameType");
	cJSON* Operator = cJSON_GetObjectItem(root, "Operator");

	//TODO错误情况检测

	if (userId == nullptr || roomId == nullptr || gameType == nullptr || Operator == nullptr)
	{
		LOG_ERROR << "CS_ReturnRoom参数错误";
		return;
	}
	//类型错误
	if (userId->type != cJSON_Number || roomId->type != cJSON_Number || Operator->type != cJSON_Number || gameType->type != cJSON_String)
	{
		LOG_ERROR << "CS_ReturnRoom参数错误";
	}

	ReturnRoom_GameCell* cell = new ReturnRoom_GameCell();
	cell->netId = this->GetNetId();
	cell->userId = userId->valueint;
	cell->roomId = roomId->valueint;
	cell->Operator = Operator->valueint;
	cell->gameType = gameType->valuestring;

	GameServer::Ins()->Post(cell);

}

void User::SC_ReturnRoom(int userId, int roomId, int Operator, const std::string& gameType)
{
	Room* room = RoomMgr::GetInstance().FindRoom(roomId);
	User** players = room->GetPlayers();
	int playerMaxNum = room->GetPlayerMaxNum();

	cJSON* SC_ReturnRoom = cJSON_CreateObject();
	cJSON_AddStringToObject(SC_ReturnRoom, "cmd", "SC_ReturnRoom");
	cJSON_AddNumberToObject(SC_ReturnRoom, "roomId", roomId);
	cJSON_AddNumberToObject(SC_ReturnRoom, "userId", userId);
	cJSON_AddNumberToObject(SC_ReturnRoom, "Operator", Operator);
	cJSON_AddStringToObject(SC_ReturnRoom, "gameType", gameType.c_str());



	if (Operator == 1 || Operator == 2)
	{
		//发给所有人
		for (int i = 0; i < playerMaxNum; i++)
		{
			if (players[i] != nullptr)
			{
				players[i]->SendJSON(SC_ReturnRoom);
			}
		}
		//清空房间游戏状态
		room->ClearGame();
	}
	else if (Operator == 0)
	{
		//只发给观战者
		SendJSON(SC_ReturnRoom);
	}

	cJSON_Delete(SC_ReturnRoom);
}