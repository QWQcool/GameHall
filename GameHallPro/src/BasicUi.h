#pragma once

class BasicUi
{
private:
	static BasicUi* instance; // 静态指针实例
	//选择游戏类别
	int _selectGameType = 0;
	//是登录UI还是注册UI
	bool _LoginOrRegister = false;
	//是否展示创造房间UI
	bool _roomCreate = false;
	//是否展示房间列表UI
	bool _roomList = false;
	//客户端初始化
	void ClientInit();
public:
	static BasicUi& GetInstance();
	void GameHallWidget();
private:
	//注册界面
	void RegisterWidget();
	//登陆界面
	void LoginWidget();

	void PreemptLoginWidget();

	//游戏房间大厅
	void GameHall();
	//游戏大厅的ui组件之创造房间
	void RoomCreateWidget();
	//房间列表更新用
	void UpdateAndSortRooms();
	//游戏房间列表的ui组件之房间列表
	void RoomList();
};
