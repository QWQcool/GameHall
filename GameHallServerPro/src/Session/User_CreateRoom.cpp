#include "../Session.h"
#include "../GameServer.h"
#include "../User.h"
#include "../RoomMgr.h"

class CreateRoom_GameCell : public IGameCell
{
public:
	uint64_t netId;
	std::string roomName;
	std::string roomGameType;
	int roomOwnerId;

public:
	virtual void Exec() override
	{
		Session* session = GameServer::Ins()->FindNetIdSession(netId);
		if (nullptr == session) {
			LOG_ERROR << "没有找到对应的 Session:" << netId;
			return;
		}
		User* user = GameServer::Ins()->FindUser(roomOwnerId);
		if (nullptr == user)
		{
			LOG_ERROR << "没有找到对应的 User:" << roomOwnerId;
			return;
		}

		user->SC_CreateRoom(roomName, roomGameType, roomOwnerId);
	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

void User::CS_CreateRoom(cJSON* root)
{
	const char* msg = cJSON_Print(root);
	LOG_INFO << "收到cJSON:\n" << msg;
	delete msg;

	cJSON* roomName = cJSON_GetObjectItem(root, "roomName");
	cJSON* roomGameType = cJSON_GetObjectItem(root, "roomGameType");
	cJSON* roomOwnerId = cJSON_GetObjectItem(root, "roomOwnerId");

	if (nullptr == roomName->valuestring || nullptr == roomGameType->valuestring || nullptr == roomOwnerId)
	{
		LOG_ERROR << "CS_CreateRoom 参数错误";
		return;
	}
	//类型错误
	if (roomOwnerId->type != cJSON_Number || roomName->type != cJSON_String || roomGameType->type != cJSON_String)
	{
		LOG_ERROR << "CS_CreateRoom 类型错误";
		return;
	}

	CreateRoom_GameCell* cell = new CreateRoom_GameCell();
	cell->netId = this->GetNetId();
	cell->roomName = roomName->valuestring;
	cell->roomGameType = roomGameType->valuestring;
	cell->roomOwnerId = roomOwnerId->valueint;

	GameServer::Ins()->Post(cell);

}

void User::SC_CreateRoom(const std::string& roomName, const std::string& roomGameType, int roomOwnerId)
{
	cJSON* sc_createRoom = cJSON_CreateObject();

	cJSON_AddStringToObject(sc_createRoom, "cmd", "SC_CreateRoom");
	cJSON_AddStringToObject(sc_createRoom, "roomName", roomName.c_str());
	cJSON_AddStringToObject(sc_createRoom, "roomGameType", roomGameType.c_str());
	cJSON_AddNumberToObject(sc_createRoom, "roomOwnerId", roomOwnerId);

	int maxNumer = 0;

	if (roomGameType == u8"五子棋")
	{
		maxNumer = 4;
	}
	else
	{
		std::cout << " 编码错误，或类型不存在" << std::endl;
	}

	int roomNumber = 0;
	cJSON_AddNumberToObject(sc_createRoom, "playerNumber", roomNumber);
	cJSON_AddNumberToObject(sc_createRoom, "playerMaxNum", maxNumer);

	cJSON_AddNumberToObject(sc_createRoom, "result", 1);
	cJSON_AddStringToObject(sc_createRoom, "msg", "CreateRoom Success");


	int roomId = RoomMgr::GetInstance().CreateRoom(maxNumer, roomName, roomGameType, roomOwnerId);
	cJSON_AddNumberToObject(sc_createRoom, "roomId", roomId);

	SendJSON(sc_createRoom);
	cJSON_Delete(sc_createRoom);
}