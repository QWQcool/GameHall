#pragma once
#include "TcpClient.hpp"
#include "Protocol.h"
#include "cJSON.h"
#include "PlayerClient.h"
#include "RoomClient.h"
#include <map>
//#include ""

class Room;
class Player;

class GameClient : public TcpClient
{
private:
	int _widgetType = 0;
	bool _isUpdateRoomList = false;

	Room* _clientRoom;
	Player* _clientPlayer;
	std::string _token;
	std::string _username;
	std::string _password;
private:
	GameClient() 
	{
		_clientRoom = nullptr;
		_clientPlayer = nullptr;
	}; // 私有构造函数
	static GameClient* instance; // 静态指针实例
	~GameClient() {};
	GameClient(const GameClient&) = delete;
	GameClient& operator=(const GameClient&) = delete;
	std::map<int, Room*> _clientRoomMgr;//通过房间ID索引查找
public:
	virtual void OnConnect() override;

	virtual int OnNetMsg(const char* buf, int len) override;

	static GameClient& GetInstance();

	std::map<int, Room*> GetRoomListMap() { return _clientRoomMgr; }

	int GetWidgetType() { return _widgetType; }
	Room* GetRoom()
	{
		if (_clientRoom)return _clientRoom;
		else return nullptr;
	}
	Player* GetPlayer()
	{
		if (_clientPlayer)return _clientPlayer;
		else return nullptr;
	}

	int GetId() { return _clientPlayer->GetUserId(); }
	std::string GetNickname() { return _clientPlayer->GetNickname(); }
	//std::string GetToken() { return _token; }
	bool GetIsUpdateRoomList() { return _isUpdateRoomList; }

	//void TokenClear() { _token.clear(); }
	void SetWidgetType(int widgetIndex) { _widgetType = widgetIndex; }
	void SetIsUpdateRoomList(bool UpdateRoomListState) { _isUpdateRoomList = UpdateRoomListState; }

	void SendMsg(const char* buf, int len);

	void cJSONSendMsg(cJSON* root);

	void SendLoginRequest(const char* username,const char* password);

	void SendTokenLoginRequest(const char* token);

	void SendSurePreemptLoginRequest();//其实也发送了token 但发送了，抢占登陆前存储在客户端的token

	void SendRegisterRequest(const char* username, const char* password,const char* nickname);

	void SendLogoutRequest();

	void SendCreateRoomRequest(const char* roomName, const char* gameType, int roomOwnerId);

	void SendRefreshRoomListRequest();

	void SendJoinRoomRequest(int roomId, int userId);

	void SendExitRoomRequest(int roomId);

	void SendMovePosRequest(int roomId, int userId, int oldPos,int newPos);

	void SendReadyPosRequest(int roomId, int userId, int pos,int readyState);

	void SendGamePlayRequest(int roomId);

	void SendGoBangDownRequest(int x, int y, int playerIndex);

	void SendAgainGameRequest(int againState);

	void SendReturnRoomRequest(int Operator);

	void SC_LoginEvent(cJSON* root);  //对应LoignRequest的非抢占登陆世界线

	void SC_TokenLoginEvent(cJSON* root);

	void SC_PreemptLoginEvent(cJSON* root); //对应LoignRequest 的抢占登陆世界线

	void SC_SurePreemptLoginEvent(cJSON* root); //抢占登陆事件

	void SC_RegisterEvent(cJSON* root);

	void SC_LogoutEvent(cJSON* root);

	void SC_CreateRoomEvent(cJSON* root);

	void SC_RefreshRoomListEvent(cJSON* root);

	void SC_JoinRoomEvent(cJSON* root);

	void SC_RoomInfoEvent(cJSON* root);

	void SC_PlayerJoinEvent(cJSON* root);

	void SC_ExitRoomEvent(cJSON* root);

	void SC_UpdatePosEvent(cJSON* root);

	void SC_UpdateReadyPosEvent(cJSON* root);

	void SC_GoBangPlayEvent(cJSON* root);

	void SC_GoBangDownEvent(cJSON* root);

	void SC_AgainGameEvent(cJSON* root);

	void SC_ReturnRoomEvent(cJSON* root);

	void SC_DeleteRoomEvent(cJSON* root);
};
