#include "../Session.h"
#include "../DBMgr.h"
#include "../GameServer.h"
#include "../User.h"

class Logout_GameCell : public IGameCell
{
public:
	uint64_t netId;
	int userId;

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

		GameServer::Ins()->RemoveUser(user);

		session->SC_Logout();
	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

void Session::CS_Logout(cJSON* root)
{
	const char* msg = cJSON_Print(root);
	LOG_INFO << "收到cJSON:\n" << msg;
	delete msg;
	cJSON* userId = cJSON_GetObjectItem(root, "userId");

	int clientUid = userId->valueint;

	Logout_GameCell* logoutGameCell = new Logout_GameCell();
	logoutGameCell->userId = clientUid;
	logoutGameCell->netId = GetNetId();

	GameServer::Ins()->Post(logoutGameCell);
	//断开连接
	//Close();

}

void Session::SC_Logout()
{
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "SC_Logout");
	SendJSON(root);
	cJSON_Delete(root);
}