#include "Session.h"
#include <functional>   // 函数对象支持
#include "User.h"       // 用户类
#include "Room.h"       // 房间类
#include "RoomMgr.h"    // 房间管理
#include "GameServer.h" // 游戏服务器主类

// 构造函数
Session::Session()
{
	static uint64_t netId = 0;      // 静态变量，保证每个Session有唯一ID
	_netId = ++netId;               // 分配网络ID
	_user = nullptr;                // 初始化用户指针为空

	// 初始化消息处理函数映射表
	_funcs["CS_Login"] = std::bind(&Session::CS_Login, this, std::placeholders::_1);
	_funcs["CS_Register"] = std::bind(&Session::CS_Register, this, std::placeholders::_1);
	_funcs["CS_Logout"] = std::bind(&Session::CS_Logout, this, std::placeholders::_1);
	_funcs["CS_SurePreemptLogin"] = std::bind(&Session::CS_SurePreemptLoginEvent, this, std::placeholders::_1);
}

// 析构函数
Session::~Session()
{
	// 目前没有需要特殊清理的资源
}

// 处理网络消息
int Session::OnNetMsg(const char* const buff, int len)
{
	if (len <= 2) return 0; // 消息长度不足，忽略

	// 解析消息长度(前2字节)
	uint16_t packSize = *((uint16_t*)buff);

	// 提取消息内容(跳过长度字段)
	std::string msg(buff + 2, packSize - 2);

	// 解析JSON
	cJSON* root = cJSON_Parse(msg.c_str());
	if (!root) {
		LOG_ERROR << "JSON解析失败";
		return packSize;
	}

	// 获取命令字段
	cJSON* cmd = cJSON_GetObjectItem(root, "cmd");

	if (cmd && cmd->type == cJSON_String) {
		// 查找对应的处理函数
		auto find = _funcs.find(cmd->valuestring);
		if (find != _funcs.end()) {
			// 找到处理函数，执行它
			LOG_INFO << "Session:收到指令 [" << cmd->valuestring << "]";
			find->second(root);
		}
		else if (_user != nullptr) {
			// 如果已登录，将消息转发给用户对象处理
			_user->OnNetMsg(root);
		}
		else {
			LOG_ERROR << "未找到相关指令 [" << cmd->valuestring << "]";
		}
	}

	// 清理JSON对象
	cJSON_Delete(root);
	return packSize; // 返回已处理的消息长度
}

// 处理断开连接
void Session::OnDisconnect()
{
	LOG_INFO << "客户端关闭";

	if (_user != nullptr) {
		// 如果有关联用户
		Room* room = _user->GetRoom();
		if (room != nullptr) {
			// 如果用户在房间中，删除房间
			int roomId = room->GetRoomId();
			GameServer::Ins()->ServerDeleteRoom(roomId);
		}

		// 从服务器移除用户
		GameServer::Ins()->RemoveUser(_user);
		_user = nullptr;
	}

	// 从服务器移除会话
	GameServer::Ins()->RemoveSession(this);
}

// 发送JSON消息
void Session::SendJSON(cJSON* root)
{
	// 将JSON转换为字符串
	const char* buf = cJSON_PrintUnformatted(root);
	int bufLen = strlen(buf);

	// 构造消息: 2字节长度 + 消息内容
	uint16_t len = bufLen + 2;
	Send((char*)&len, 2);    // 发送长度
	Send(buf, bufLen);        // 发送内容

	// 释放JSON字符串内存
	free((void*)buf);
}