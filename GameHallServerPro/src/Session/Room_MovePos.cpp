#include "../Session.h"
#include "../GameServer.h"
#include "../User.h"
#include "../RoomMgr.h"
#include "../Room.h"

class MovePos_GameCell : public IGameCell
{
public:
	uint64_t netId;
	int userId;
	int roomId;
	int oldPos;
	int newPos;
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

		room->SC_MovePos(userId,roomId,oldPos,newPos);
	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

void Room::CS_MovePos(cJSON* root)
{
	const char* msg = cJSON_Print(root);
	LOG_INFO << "收到cJSON:\n" << msg;
	delete msg;

	cJSON* roomId = cJSON_GetObjectItem(root, "roomId");
	cJSON* userId = cJSON_GetObjectItem(root, "userId");
	cJSON* oldPos = cJSON_GetObjectItem(root, "oldPos");
	cJSON* newPos = cJSON_GetObjectItem(root, "newPos");

	if (nullptr == roomId || nullptr == userId|| nullptr == oldPos || nullptr == newPos)
	{
		LOG_ERROR << "CS_MovePos 参数错误";
		return;
	}

	//类型错误
	if (cJSON_Number != roomId->type || cJSON_Number != userId->type || cJSON_Number != oldPos->type || cJSON_Number != newPos->type)
	{
		LOG_ERROR << "CS_MovePos 类型错误";
		return;
	}

	// Validate positions
	if (oldPos->valueint < 0 || oldPos->valueint >= _playerMaxNum || newPos->valueint < 0 || newPos->valueint >= _playerMaxNum)
	{
		std::cout << "Invalid positions" << std::endl;
		return;
	}

	// If the new position is the same as the old position, do nothing
	if (oldPos == newPos)
	{
		std::cout << "Invalid positions" << std::endl;
		return;
	}

	int clientUserId = userId->valueint;
	int clientRoomId = roomId->valueint;

	MovePos_GameCell* cell = new MovePos_GameCell();
	cell->netId = this->GetNetId(clientUserId);
	cell->userId = clientUserId;
	cell->roomId = clientRoomId;
	cell->oldPos = oldPos->valueint;
	cell->newPos = newPos->valueint;

	GameServer::Ins()->Post(cell);

}

void Room::SC_MovePos(int userId,int roomId,int oldPos,int newPos)
{
	cJSON* SC_MovePos = cJSON_CreateObject();

	//如果新位置非空,拒绝移动
	if (_players[newPos])
	 {
	 	LOG_INFO << "New Positions Existence Player";
	 	return;
	 }
	// Move the player objects in the array
	User* temp = _players[oldPos];
	_players[oldPos] = nullptr;
	_players[newPos] = temp;

	cJSON_AddStringToObject(SC_MovePos, "cmd", "SC_UpdatePos");
	cJSON_AddNumberToObject(SC_MovePos, "roomId", roomId);
	cJSON_AddNumberToObject(SC_MovePos, "userId", userId);
	cJSON_AddNumberToObject(SC_MovePos, "oldPos", oldPos);
	cJSON_AddNumberToObject(SC_MovePos, "newPos", newPos);

	// Send the message to all players
	cJSON* players = cJSON_CreateArray();
	for (int i = 0; i < _playerMaxNum; i++)
	{
		if (_players[i] != nullptr)
		{
			cJSON* player = cJSON_CreateObject();
			cJSON_AddItemToObject(player, "pos", cJSON_CreateNumber(i));
			cJSON_AddItemToObject(player, "userId", cJSON_CreateNumber(_players[i]->GetUserId()));
			cJSON_AddItemToObject(player, "nickname", cJSON_CreateString(_players[i]->GetNickname().c_str()));
			cJSON_AddItemToArray(players, player);
		}
	}
	cJSON_AddItemToObject(SC_MovePos, "players", players);

	for (int i = 0; i < _playerMaxNum; i++)
	{
		if (_players[i] != nullptr)
		{
			_players[i]->SendJSON(SC_MovePos);
		}
	}
	cJSON_Delete(SC_MovePos);
}

