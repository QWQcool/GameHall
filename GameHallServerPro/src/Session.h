//
// Created by xmr on 2024/8/7.
//

#ifndef GAMESERVER_SESSION_H
#define GAMESERVER_SESSION_H

//功能详解
//​​会话管理​​：
//每个Session代表一个客户端连接
//使用唯一的_netId标识每个连接
//可以关联一个User对象表示已登录用户
//​​消息处理​​：
//使用JSON格式进行通信
//消息格式：2字节长度 + JSON内容
//通过"cmd"字段区分不同类型的消息
//使用函数映射表(_funcs)动态路由消息到对应的处理函数
//​​主要功能​​：
//用户注册(CS_Register / SC_Register)
//用户登录(CS_Login / SC_Login)
//用户登出(CS_Logout / SC_Logout)
//抢占登录处理(SC_PreemptLoginEvent / CS_SurePreemptLoginEvent / SC_PreemptLoginSureEvent)
//​​异常处理​​：
//连接断开时自动清理资源
//未登录用户只能处理有限的命令
//已登录用户的消息会转发给User对象处理
//​​房间管理​​：
//用户断开连接时，如果用户在房间中，会自动删除房间
//设计特点
//​​基于JSON的协议​​：易于扩展和调试
//​​命令路由机制​​：通过映射表自动路由消息到处理函数
//​​资源自动管理​​：连接断开时自动清理相关资源
//​​模块化设计​​：Session负责连接管理，User负责业务逻辑
//这个Session类是游戏服务器网络层的核心组件，负责维护客户端连接、消息收发和基本的用户生命周期管理。

#include "XNet.h"           // 网络基础库
#include <unordered_map>    // 哈希表容器
#include "cJSON.h"          // JSON解析库

class User;
struct UserInfo;

// Session类继承自X::TcpSocket，表示一个TCP连接会话
class Session : public X::TcpSocket {
public:
	Session();      // 构造函数
	~Session();     // 析构函数

	// 重写父类方法
	int OnNetMsg(const char* const buff, int len) override;    // 处理网络消息
	void OnDisconnect() override;                               // 处理断开连接

public:
	uint64_t GetNetId() { return _netId; }          // 获取网络ID
	void SetUser(User* user) { _user = user; }      // 设置关联用户

public:
	// JSON消息发送和各类消息处理函数
	void SendJSON(cJSON* root);                     // 发送JSON消息

	// 注册相关
	void CS_Register(cJSON* root);                  // 处理客户端注册请求
	void SC_Register(bool status);                  // 发送注册响应

	// 登录相关
	void CS_Login(cJSON* root);                     // 处理客户端登录请求
	void SC_Login(bool status);                     // 发送登录响应

	// 登出相关
	void CS_Logout(cJSON* root);                    // 处理客户端登出请求
	void SC_Logout();                               // 发送登出响应

	// 抢占登录相关
	void SC_PreemptLoginEvent(std::string& username, std::string& password); // 发送抢占登录事件
	void CS_SurePreemptLoginEvent(cJSON* root);     // 处理客户端确认抢占登录
	void SC_PreemptLoginSureEvent(bool bStatus, UserInfo userInfo); // 发送抢占登录确认结果

	User* GetUser() { return _user; }               // 获取关联用户

private:
	std::unordered_map<std::string, std::function<void(cJSON* root)>> _funcs; // 消息处理函数映射表
	uint64_t _netId;     // 网络连接唯一ID
	User* _user;         // 关联的用户对象
};

#endif //GAMESERVER_SESSION_H
