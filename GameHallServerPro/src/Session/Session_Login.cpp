#include "../Session.h"
#include "../DBMgr.h"
#include "Utils.h"
#include "SnowFlake.hpp"
#include "../GameServer.h"
#include "../User.h"
#include "../Room.h"
#include "../GameBase.hpp"
#include "../RoomMgr.h"

class Login_GameCell : public IGameCell 
{
public:
	bool bStatus;
	uint64_t netId;
	UserInfo userInfo;
public:
	virtual void Exec() override 
	{
		Session* session = GameServer::Ins()->FindNetIdSession(netId);
		if (nullptr == session) {
			LOG_ERROR << "没有找到对应的 Session:" << netId;
			return;
		}

		User* user = GameServer::Ins()->FindUser(userInfo.uid);
		if (nullptr != user) 
		{
			//重复登录
			session->SC_PreemptLoginEvent(userInfo.username,userInfo.password);
		}
		else 
		{
			// 创建新用户
			user = new User();
			user->Init(userInfo);
			user->SetSession(session);
			session->SetUser(user);
			session->SC_Login(bStatus);
			GameServer::Ins()->AddUser(user);
		}
	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

class Login_DBCell : public IDBCell {
public:
	std::string username;
	std::string password;
	uint64_t netId;

	virtual void DBExec(X::DB::DBSqlite* pSqlite) override 
	{
		const char* sql = "SELECT uid,userName,password,nickname FROM tb_users WHERE username = ?;";
		char* pError = NULL;
		Login_GameCell* loginGameCell = new Login_GameCell();
		loginGameCell->bStatus = false;
		loginGameCell->netId = netId;

		if (pSqlite->BeginPrecompiled(sql, &pError)) {
			pSqlite->PrecompiledBind(1, username.c_str(), username.length());
			if (X::DB::Sqlite3Enum::SQLITE_ROW == pSqlite->StepPrecompiled()) {
				pSqlite->PrecompiledGetValue(0, loginGameCell->userInfo.uid);
				pSqlite->PrecompiledGetValue(1, loginGameCell->userInfo.username);
				pSqlite->PrecompiledGetValue(2, loginGameCell->userInfo.password);
				pSqlite->PrecompiledGetValue(3, loginGameCell->userInfo.nickname);
				if (loginGameCell->userInfo.password == password)
					loginGameCell->bStatus = true;
			}
			pSqlite->EndPrecompiled();
		}

		GameServer::Ins()->Post(loginGameCell);
	}

	virtual void Release() override
	{
		IDBCell::Release();
	}
};

void Session::CS_Login(cJSON* root) 
{

	const char* msg = cJSON_Print(root);
	LOG_INFO << "收到cJSON:\n" << msg;
	delete msg;

	cJSON* username = cJSON_GetObjectItem(root, "username");
	cJSON* password = cJSON_GetObjectItem(root, "password");
	if (username == nullptr || password == nullptr)
	{
		LOG_ERROR<< "username or password is null";
		return;
	}

	if (username->type != cJSON_String || password->type != cJSON_String)
	{
		LOG_ERROR<< "username or password is not string";
		return;
	}

	Login_DBCell* cell = new Login_DBCell();
	cell->username = username->valuestring;
	cell->password = password->valuestring;
	cell->netId = GetNetId();
	DBMgr::Post(cell);
}

void Session::SC_Login(bool status) 
{
	const UserInfo& info = _user->GetUserInfo();
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "SC_Login");
	cJSON_AddItemToObject(root, "status", cJSON_CreateBool(status));
	if (status) 
	{
		cJSON_AddStringToObject(root, "msg", "Login Success");

		cJSON* player = cJSON_CreateObject();
		cJSON_AddStringToObject(player, "username", info.username.c_str());
		cJSON_AddStringToObject(player, "nickname", info.nickname.c_str());
		cJSON_AddNumberToObject(player, "playerId", info.uid);

		cJSON_AddItemToObject(root, "player", player);
	}
	SendJSON(root);
	cJSON_Delete(root);
}

void Session::SC_PreemptLoginEvent(std::string& username,std::string& password)
{
	cJSON* SC_PreemptLoginEvent = cJSON_CreateObject();

	cJSON_AddStringToObject(SC_PreemptLoginEvent, "cmd", "SC_PreemptLogin");
	cJSON_AddStringToObject(SC_PreemptLoginEvent, "username", username.c_str());
	cJSON_AddStringToObject(SC_PreemptLoginEvent, "password", password.c_str());
	cJSON_AddNumberToObject(SC_PreemptLoginEvent, "result", 1);
	cJSON_AddStringToObject(SC_PreemptLoginEvent, "msg", "SC_PreemptLoginEvent");

	SendJSON(SC_PreemptLoginEvent);
	cJSON_Delete(SC_PreemptLoginEvent);
}

class SurePreemptLogin_GameCell : public IGameCell
{
public:
	bool bStatus;
	uint64_t netId;
	UserInfo userInfo;
public:
	virtual void Exec() override
	{
		Session* session = GameServer::Ins()->FindNetIdSession(netId);
		if (nullptr == session) 
		{
			LOG_ERROR << "没有找到对应的 Session:" << netId;
			return;
		}
		session->SC_PreemptLoginSureEvent(bStatus, userInfo);

	}

	virtual void Release() override
	{
		IGameCell::Release();
	}
};

class SurePreemptLogin_DBCell : public IDBCell {
public:
	std::string username;
	std::string password;
	uint64_t netId;

	virtual void DBExec(X::DB::DBSqlite* pSqlite) override
	{
		const char* sql = "SELECT uid,userName,password,nickname FROM tb_users WHERE username = ?;";
		char* pError = NULL;
		SurePreemptLogin_GameCell* cell = new SurePreemptLogin_GameCell();
		cell->bStatus = false;
		cell->netId = netId;

		if (pSqlite->BeginPrecompiled(sql, &pError)) {
			pSqlite->PrecompiledBind(1, username.c_str(), username.length());
			if (X::DB::Sqlite3Enum::SQLITE_ROW == pSqlite->StepPrecompiled()) {
				pSqlite->PrecompiledGetValue(0, cell->userInfo.uid);
				pSqlite->PrecompiledGetValue(1, cell->userInfo.username);
				pSqlite->PrecompiledGetValue(2, cell->userInfo.password);
				pSqlite->PrecompiledGetValue(3, cell->userInfo.nickname);
				if (cell->userInfo.password == password)
					cell->bStatus = true;
			}
			pSqlite->EndPrecompiled();
		}

		GameServer::Ins()->Post(cell);
	}

	virtual void Release() override
	{
		IDBCell::Release();
	}
};

void Session::CS_SurePreemptLoginEvent(cJSON* root)
{
	cJSON* SurePreemptLogin = cJSON_CreateObject();
	cJSON_AddStringToObject(SurePreemptLogin, "cmd", "CS_SurePreemptLogin");
	cJSON* username = cJSON_GetObjectItem(root, "username");
	cJSON* password = cJSON_GetObjectItem(root, "password");
	if (username == nullptr||password == nullptr)
	{
		LOG_ERROR<< "username is null";
		return;
	}
	if (username->type != cJSON_String||password->type != cJSON_String)
	{
		LOG_ERROR<< "username is not string";
		return;
	}

	SurePreemptLogin_DBCell* cell = new SurePreemptLogin_DBCell();
	cell->username = username->valuestring;
	cell->password = password->valuestring;
	cell->netId = GetNetId();
	DBMgr::Post(cell);
}

void Session::SC_PreemptLoginSureEvent(bool bStatus, UserInfo userInfo)
{
	if (bStatus)
	{
		cJSON* SC_PreemptLogin = cJSON_CreateObject();
		cJSON_AddStringToObject(SC_PreemptLogin, "cmd", "SC_PreemptLoginSure");

		int isInGame = 0;
		int isInRoom = 0;

		User* oldUser = GameServer::Ins()->FindUser(userInfo.uid);
		Session* oldSession = oldUser->GetSession();
		Room* oldRoom = oldUser->GetRoom();
		int playerPos = -1;
		int roomId = -1;

		if (oldSession!=nullptr && oldUser!=nullptr) 
		{
			this->SetUser(oldUser);
			oldUser->SetSession(this);
			oldSession->SetUser(nullptr);
			cJSON* SC_ClientClose = cJSON_CreateObject();
			cJSON_AddStringToObject(SC_ClientClose, "cmd", "SC_ClientClose");
			cJSON_AddStringToObject(SC_ClientClose, "msg", "Client Close");
			cJSON_AddNumberToObject(SC_ClientClose, "result", 1);
			oldSession->SendJSON(SC_ClientClose);
			cJSON_Delete(SC_ClientClose);

			if (oldRoom != nullptr)
			{
				isInRoom = 1;
				int playerMaxNum = oldRoom->GetPlayerNum();
				playerPos = -1;
				User** palyers = oldRoom->GetPlayers();
				for (int i = 0; i < playerMaxNum; i++)
				{
					if (palyers[i]->GetUserId() == oldUser->GetUserId())
					{
						playerPos = i;
						break;
					}
				}

				roomId = oldRoom->GetRoomId();
				std::map<int, Room*> roomMap = RoomMgr::GetInstance().GetRoomListMap();
				cJSON* roomsArray = cJSON_CreateArray(); // 创建房间数组

				std::string roomGameType = oldRoom->GetRoomGameType();

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

					cJSON_AddItemToArray(roomsArray, room); // 添加到数组
				}
				cJSON_AddItemToObject(SC_PreemptLogin, "rooms", roomsArray);
				GameBase* game = oldRoom->GetGameBase();
				if (game != nullptr && roomGameType == u8"五子棋")
				{
					isInGame = 1;
					//TODO:
					//在游戏状态中，获取游戏信息
				}
			}

		}

		int jsonUid = oldUser->GetUserId();
		const std::string& jsonUsername = oldUser->GetUsername();

		cJSON_AddNumberToObject(SC_PreemptLogin, "roomId", roomId);
		cJSON_AddNumberToObject(SC_PreemptLogin, "userId", jsonUid);
		cJSON_AddStringToObject(SC_PreemptLogin, "nickname", jsonUsername.c_str());
		cJSON_AddNumberToObject(SC_PreemptLogin, "isInRoom", isInRoom);
		cJSON_AddNumberToObject(SC_PreemptLogin, "isInGame", isInGame);
		cJSON_AddNumberToObject(SC_PreemptLogin, "result", 1);
		cJSON_AddStringToObject(SC_PreemptLogin, "msg", "SurePreemptLogin Success");

		SendJSON(SC_PreemptLogin);

		if (oldRoom != nullptr)
		{
			oldRoom->SendRoomInfo(playerPos);
		}

		cJSON_Delete(SC_PreemptLogin);
	}
	else
	{
		LOG_DEBUG << "error SC_SurePreemptLoginRespond cmd";
		return;
	}
}
