#pragma once
#include<iostream>
//暂定，未使用中
#define REGIST_OK "regist ok"
#define REGIST_FAILED "regist failed"
#define LOGIN_OK "login ok"
#define LOGIN_FAILED "login failed"
#define UNKNOWN_ERROR "unknown error"                    // 通用未知错误
enum class ENUM_MSG_TYPE {
	ENUM_MSG_TYPE_MIN = 0,
	ENUM_MSG_TYPE_REGIST_REQUEST, //注册请求
	ENUM_MSG_TYPE_REGIST_RESPOND, //注册回复
	ENUM_MSG_TYPE_LOGIN_REQUEST,  //登录请求
	ENUM_MSG_TYPE_LOGIN_RESPOND, //登陆回复
	ENUM_MSG_TYPE_JOIN_ROOM_REQUEST, //加入房间请求
	ENUM_MSG_TYPE_JOIN_ROOM_RESPOND, //加入房间回复
	ENUM_MSG_TYPE_DOWN_REQUEST,//下棋请求
	ENUM_MSG_TYPE_DOWN_RESPOND,//下棋回复
	ENUM_MSG_TYPE_MAX = 0x00ffffff //u int 最大值
};

//展示的ui类别
enum GameType
{
	Game_Login=0,
	Game_Hall,
	Game_Room,
	Preempt_Login,
	Game_OtherState,
};

class GameBase
{
private:
	std::string _gameType;
public:
	virtual void Init() = 0;
	virtual void ShowWidget() = 0;
	virtual void ClearGame() = 0;
};