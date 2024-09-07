#include "BasicUi.h"
#include "GameClient.h"
#include "ImguiAll.h"
#include "Protocol.h"
#include "cJSON.h"
#include <iostream>
#include <algorithm>

//UI图片存储
static void* sg_LoginBg = nullptr;
static void* sg_RegisterBg = nullptr;
static void* sg_PreemptLoginBg = nullptr;
static void* sg_GameHallBg = nullptr;

//房间列表管理数组
ImVector<Room*> sg_roomInfos;

// 静态成员变量定义
BasicUi* BasicUi::instance = nullptr;

void BasicUi::ClientInit()
{
	static bool init = false;
	if (init)
	{
		return;
	}
	init = true;

	sg_LoginBg = LoadImageEx("./src/Image/LoginImg.jpg");
	sg_RegisterBg = LoadImageEx("./src/Image/RegisterImg.jpg");
	sg_PreemptLoginBg = LoadImageEx("./src/Image/PreemptLoginImg.jpg");
	sg_GameHallBg = LoadImageEx("./src/Image/GameHallImg.jpg");
	GameClient::GetInstance().Connect("127.0.0.1", 9999);
}

BasicUi& BasicUi::GetInstance()
{
	if (instance == nullptr) {
		instance = new BasicUi();
	}
	return *instance;
}

void BasicUi::GameHallWidget()
{
	GameClient::GetInstance().LoopOnce();
	ClientInit();
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();

	//ImGui::SetNextWindowPos(ImVec2(0, 40));
	//窗口居中出现
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(960, 720));//960宽,720高
	switch (GameClient::GetInstance().GetWidgetType())
	{
	case Game_Login:
	{
		if (_LoginOrRegister)
		{
			RegisterWidget();
		}
		else
		{
			LoginWidget();
		}
		break;
	}
	case Game_Hall:
	{
		GameHall();
		break;
	}
	case Game_Room:
	{
		GameClient::GetInstance().GetRoom()->ShowWidget();
		break;
	}
	case Preempt_Login:
	{
		PreemptLoginWidget();
		break;
	}
	default:
		break;
	}
}

//注册界面UI
void BasicUi::RegisterWidget()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse;

	ImGui::Begin(u8"神秘的游戏大厅入口", nullptr, window_flags);

	float availableWidth = ImGui::GetContentRegionAvail().x * 0.5;
	ImVec2 windowSize = ImGui::GetWindowSize();

	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::Image(sg_RegisterBg, ImVec2(windowSize.x, windowSize.y));

	// 设置一个半透明的矩形作为背景，以确保文本和输入框可见
	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.15f, windowSize.y * 0.2f));  // 设置位置
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));  // 设置颜色
	ImGui::BeginChild("##registerbox", ImVec2(windowSize.x * 0.7f, windowSize.y * 0.4f), true);  // 创建子窗口
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

	ImGui::SeparatorText(u8"注册");  // 显示分割线

	static char Nickname[64] = "";
	ImGui::SetCursorPos(ImVec2(50, 50));
	ImGui::Text("Nickname: ");
	ImGui::SameLine();
	ImGui::InputTextMultiline("##Nickname", Nickname, IM_ARRAYSIZE(Nickname), ImVec2(availableWidth - 2, 25));
	ImGui::SetCursorPos(ImVec2(200, 260));

	static char Username[64] = "";
	ImGui::SetCursorPos(ImVec2(50, 100));
	ImGui::Text("Username: ");
	ImGui::SameLine();
	ImGui::InputTextMultiline("##Username", Username, IM_ARRAYSIZE(Username), ImVec2(availableWidth - 2, 25));
	ImGui::SetCursorPos(ImVec2(50, 150));

	static char Password[64] = "";
	ImGui::Text("Password : ");
	ImGui::SameLine();
	ImGui::InputTextMultiline("##Password", Password, IM_ARRAYSIZE(Password), ImVec2(availableWidth, 25), ImGuiInputTextFlags_Password);
	ImGui::SetCursorPos(ImVec2(50, 200));
	if (ImGui::Button(u8"注册"))
	{
		GameClient::GetInstance().SendRegisterRequest(Username, Password, Nickname);
		//g_widgetType = Game_Hall;
	}
	ImGui::SetItemTooltip("I am a Register");

	ImGui::SetCursorPos(ImVec2(500, 200));
	ImGui::Toggle(
		_LoginOrRegister ? "To Login" : "To Register",
		&_LoginOrRegister);
	ImGui::PopStyleColor();

	ImGui::EndChild();  // 结束子窗口
	ImGui::End();
}

//登陆页面UI
void BasicUi::LoginWidget()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse;

	ImGui::Begin(u8"神秘的游戏大厅入口", nullptr, window_flags);

	float availableWidth = ImGui::GetContentRegionAvail().x * 0.5;
	ImVec2 windowSize = ImGui::GetWindowSize();

	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::Image(sg_LoginBg, ImVec2(windowSize.x, windowSize.y));

	// 设置一个半透明的矩形作为背景，以确保文本和输入框可见
	ImGui::SetCursorPos(ImVec2(windowSize.x*0.15f, windowSize.y*0.2f));  // 设置位置
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));  // 设置颜色
	ImGui::BeginChild("##loginbox", ImVec2(windowSize.x*0.7f, windowSize.y*0.4f), true);  // 创建子窗口
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.1f, 0.1f, 1.0f));

	ImGui::SeparatorText(u8"登陆");  // 显示分割线

	//ImVec2 childSize = ImGui::GetContentRegionAvail();
	//656 244

	static char Username[64] = "";
	ImGui::SetCursorPos(ImVec2(50, 100));
	ImGui::Text("Username: ");
	ImGui::SameLine();
	ImGui::InputTextMultiline("##Username", Username, IM_ARRAYSIZE(Username), ImVec2(availableWidth - 2, 25));
	ImGui::SetCursorPos(ImVec2(50, 150));

	static char Password[64] = "";
	ImGui::Text("Password : ");
	ImGui::SameLine();
	ImGui::InputTextMultiline("##Password", Password, IM_ARRAYSIZE(Password), ImVec2(availableWidth, 25), ImGuiInputTextFlags_Password);
	ImGui::SetCursorPos(ImVec2(50, 200));
	if (ImGui::Button(u8"登陆"))
	{
		GameClient::GetInstance().SendLoginRequest(Username, Password);
		//g_widgetType = Game_Hall;
	}
	ImGui::SetItemTooltip("I am a LoginButton");

	ImGui::SetCursorPos(ImVec2(500, 200));
	ImGui::Toggle(
		_LoginOrRegister ? "To Login" : "To Register",
		&_LoginOrRegister);

	ImGui::PopStyleColor();

	ImGui::EndChild();  // 结束子窗口
	ImGui::End();
}

void BasicUi::PreemptLoginWidget()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse;

	ImGui::Begin(u8"神秘的游戏大厅入口", nullptr, window_flags);

	// 计算房间列表子窗口的位置和大小
	ImVec2 windowSize = ImGui::GetWindowSize();

	float availableWidth = ImGui::GetContentRegionAvail().x * 0.5;

	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::Image(sg_PreemptLoginBg, ImVec2(windowSize.x, windowSize.y));

	ImGui::SetCursorPos(ImVec2(windowSize.x*0.1f, windowSize.y*0.1f));  // 设置位置
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));  // 设置颜色
	ImGui::BeginChild("##loginbox", ImVec2(windowSize.x*0.3f, windowSize.y*0.4f), true);  // 创建子窗口
	ImGui::PopStyleColor();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::SeparatorText(u8"抢占登陆");
	ImGui::SetCursorPos(ImVec2(50, 100));
	ImGui::Text(u8"用户已经在线，你确定要抢占登陆吗？");

	ImGui::SetCursorPos(ImVec2(50, 150));
	if (ImGui::Button(u8"抢占登陆"))
	{
		GameClient::GetInstance().SendSurePreemptLoginRequest();
	}
	ImGui::SetItemTooltip(u8"ForceLoginTips");

	ImGui::SetCursorPos(ImVec2(150, 150));
	if (ImGui::Button(u8"返回登陆页面"))
	{
		GameClient::GetInstance().SetWidgetType(Game_Login);
	}
	ImGui::PopStyleColor();
	ImGui::EndChild();  // 结束子窗口
	ImGui::End();
}

//房间大厅UI
void BasicUi::GameHall()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse;

	ImGui::Begin(u8"游戏大厅", nullptr, window_flags);

	// 计算房间列表子窗口的位置和大小
	ImVec2 windowSize = ImGui::GetWindowSize();

	float availableWidth = ImGui::GetContentRegionAvail().x;

	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::Image(sg_GameHallBg, ImVec2(windowSize.x, windowSize.y));

	int clientUserId = GameClient::GetInstance().GetPlayer()->GetUserId();
	std::string clientUserName = GameClient::GetInstance().GetPlayer()->GetNickname();

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.05f, windowSize.y * 0.05f));
	ImGui::Text(u8"用户ID: %d", clientUserId);
	ImGui::SameLine();
	if (ImGui::Button(u8"退出登录"))
	{
		GameClient::GetInstance().SendLogoutRequest();
	}
	//ImGui::Text("UserID:");
	ImGui::SameLine(availableWidth * 0.8);
	if (ImGui::Button(u8"房间列表"))
	{
		_roomList = _roomList ? false : true;//如果true变为false，如果false变为true
	}
	ImGui::SameLine();
	// Refresh room list UI
	if (ImGui::Button(u8"刷新房间列表"))
	{
		//向服务端请求更新房间列表，同时更新本地客户端存储房间列表的数组
		GameClient::GetInstance().SendRefreshRoomListRequest();
	}

	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.05f, windowSize.y * 0.1f));
	ImGui::Text(u8"昵称: %s", GameClient::GetInstance().GetPlayer()->GetNickname().c_str());
	ImGui::SameLine(availableWidth * 0.8);
	if (ImGui::Button(u8"创造房间"))
	{
		_roomCreate = _roomCreate ? false : true;
	}
	ImGui::SetCursorPos(ImVec2(0, windowSize.y * 0.12f));
	ImGui::SeparatorText(u8"分割线"); // 添加一个分割线，使UI看起来更整洁
	ImGui::PopStyleColor();
	ImGui::Spacing();   // 添加一些额外的空间
	// 如果g_roomCreate为真，则显示RoomCreateWidget的UI元素
	if (_roomCreate)
	{
		RoomCreateWidget();
	}
	if (_roomList)
	{
		RoomList();
	}
	ImGui::End();
}

//创造房间的一个小组件UI
void BasicUi::RoomCreateWidget()
{
	//UpdateAndSortRooms();
	float availableWidth = ImGui::GetContentRegionAvail().x * 0.8;

	// 计算房间列表子窗口的位置和大小
	ImVec2 windowSize = ImGui::GetWindowSize();

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6078f, 0.6824f, 0.7843f, 1.0f));
	static char RoomName[64] = "";
	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.05f, windowSize.y * 0.15f));
	ImGui::Text(u8"房名: ");
	ImGui::SameLine();
	ImGui::InputTextMultiline("##RoomName", RoomName, IM_ARRAYSIZE(RoomName), ImVec2(availableWidth*0.3, 25));

	//游戏选择选项
	static const char* gameTypes[] = { u8"五子棋", u8"五子棋(人机对战开发中)", u8"魔塔(开发中)" }; // 选项列表

	// 计算第一个按钮的起始位置
	float nextOneButtonStart = availableWidth;
	ImGui::SameLine();
	if (ImGui::Button(u8"创造"))
	{
		if (RoomName == "")return;
		GameClient::GetInstance().SendCreateRoomRequest(RoomName, gameTypes[_selectGameType], GameClient::GetInstance().GetPlayer()->GetUserId());
		// 使用 memset 清空整个数组
		memset(RoomName, 0, sizeof(RoomName));
	}
	ImGui::SameLine();
	if (ImGui::Button(u8"关闭创造UI"))
	{
		_roomCreate = false;
	}


	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.05f, windowSize.y * 0.2f));
	ImGui::Text("GameType: ");
	ImGui::SameLine();
	//添加三个互斥的选项
	for (int i = 0; i < IM_ARRAYSIZE(gameTypes); i++)
	{
		bool isSelected = (_selectGameType == i);
		if (ImGui::RadioButton(gameTypes[i], &_selectGameType, i))
		{
			// 当选项被选中时，还可以在这里添加额外的逻辑
		}
		if (i + 1 < IM_ARRAYSIZE(gameTypes)) ImGui::SameLine(); // 如果不是最后一个选项，就换到下一行
	}
	ImGui::PopStyleColor();
}

//房间列表数组更新用
void BasicUi::UpdateAndSortRooms()
{
	// 先清空旧的数据
	sg_roomInfos.clear();

	std::map<int, Room*> _clientRoomMgr = GameClient::GetInstance().GetRoomListMap();
	// 将 map 中的数据复制到 vector 中
	for (const auto& pair : _clientRoomMgr)
	{
		sg_roomInfos.push_back(pair.second);
	}

	// 对 vector 按房间号升序排序
	std::sort(sg_roomInfos.begin(), sg_roomInfos.end(), [](const Room* lhs, const Room* rhs)
		{
			return lhs->GetRoomId() < rhs->GetRoomId();
		});
}
//房间列表相关UI
void BasicUi::RoomList()
{
	static int currentPage = 0; // 当前页面
	const int itemsPerPage = 10; // 每页的房间数量

	// Options
	static ImGuiTableFlags flags =
		ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersOuterV;
	// 计算房间列表子窗口的位置和大小
	ImVec2 windowSize = ImGui::GetWindowSize();
	// windowsSize.x = 960 windowSize.y = 720 
	//std::cout<<"windowSize.x:"<<windowSize.x<<"windowSize.y:"<<windowSize.y<<std::endl;
	ImVec2 subWindowSize(windowSize.x * 0.7f, windowSize.y * 0.5f); 
	ImVec2 subWindowPos(windowSize.x * 0.15f, windowSize.y * 0.28f); 

	// 房间列表UI
	ImGui::SetCursorPos(subWindowPos); // 设置子窗口的位置
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.9f, 1.0f, 1.0f, 0.75f));  // 设置颜色
	if (ImGui::BeginChild("RoomList", subWindowSize, true))
	{
	ImGui::PopStyleColor();
		//房间列表UI
		if (ImGui::BeginTable("table_sorting", 6, flags, ImVec2(0.0f, 1 * 10), 0.0f))
		{
			// Setup table columns
			ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthStretch, 0.0f, 0); // 固定宽度
			ImGui::TableSetupColumn("RoomName", ImGuiTableColumnFlags_WidthStretch, 0.0f, 1); // 自动宽度
			ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthStretch, 0.0f, 2); // 固定宽度
			ImGui::TableSetupColumn("RoomPeople", ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthStretch, 0.0f, 3); // 自动宽度
			ImGui::TableSetupColumn("GameType", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthStretch, 0.0f, 4); // 自动宽度
			ImGui::TableSetupColumn("OwnerId", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthStretch, 0.0f, 5); // 自动宽度
			ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
			//标题行的显示
			//ImGui::TableHeadersRow();

			int startIndex = currentPage * itemsPerPage;  //当前页第一个元素索引
			int finallyIndex = (currentPage + 1) * itemsPerPage; //当前页最后一个元素索引+1
			// 确保finallyIndex不超过items的大小
			if (finallyIndex > sg_roomInfos.size())finallyIndex = sg_roomInfos.size();

			// Display the room items
			for (int row_n = startIndex; row_n < finallyIndex; row_n++)
			{
				// Display a data item
				Room* item = sg_roomInfos[row_n];
				ImGui::PushID(item->GetRoomId());
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::Text("%04d", item->GetRoomId());

				ImGui::TableNextColumn();
				ImGui::TextUnformatted(item->GetRoomName().c_str());

				ImGui::TableNextColumn();
				bool isFull = item->GetPlayerNum() >= item->GetPlayerMaxNum();
				if (isFull)
				{
					ImGui::BeginDisabled();
				}
				if (ImGui::SmallButton(isFull ? u8"满人" : u8"加入"))
				{
					if (!isFull)
					{
						// Join logic here
						GameClient::GetInstance().SendJoinRoomRequest(item->GetRoomId(), GameClient::GetInstance().GetPlayer()->GetUserId());
					}
				}
				if (isFull)
				{
					ImGui::EndDisabled();
				}

				ImGui::TableNextColumn();
				ImGui::Text("%d/%d", item->GetPlayerNum(), item->GetPlayerMaxNum());

				ImGui::TableNextColumn();
				ImGui::Text(u8"%s", item->GetRoomGameType().c_str());

				ImGui::TableNextColumn();
				ImGui::Text("%d", item->GetRoomOwner());

				ImGui::PopID();
			}

			// Fill remaining rows
			for (int row_n = finallyIndex; row_n < startIndex + itemsPerPage; row_n++)
			{
				ImGui::PushID(row_n);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text(""); // Placeholder text
				ImGui::TableNextColumn();
				ImGui::Text("");
				ImGui::TableNextColumn();
				ImGui::Text("");// Empty button
				ImGui::TableNextColumn();
				ImGui::Text("");
				ImGui::TableNextColumn();
				ImGui::Text("");
				ImGui::TableNextColumn();
				ImGui::Text("");
				ImGui::PopID();
			}
			ImGui::EndTable();
		}

		// Page navigation UI
		// 页面切换按钮UI
		bool isFirstPage = (currentPage == 0);
		bool isLastPage = (currentPage >= sg_roomInfos.Size / itemsPerPage);

		// 计算按钮居中的位置
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10); // 添加一些间距
		ImGui::SetCursorPosX((subWindowSize.x - ImGui::CalcItemWidth()) * 0.75f);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9804f, 0.6941f, 0.8078f, 1.0f));

		if (isFirstPage)
			ImGui::BeginDisabled();
		if (ImGui::Button(u8"第一页"))
		{
			currentPage = 0;
		}
		if (isFirstPage)
			ImGui::EndDisabled();

		ImGui::SameLine();
		if (isFirstPage)
			ImGui::BeginDisabled();
		if (ImGui::Button(u8"上一页"))
		{
			currentPage--;
		}
		if (isFirstPage)
			ImGui::EndDisabled();

		ImGui::SameLine();
		ImGui::Text(u8"页数 %d/%d", currentPage + 1,
			(sg_roomInfos.Size + itemsPerPage - 1) / itemsPerPage == 0 ? 1: (sg_roomInfos.Size + itemsPerPage - 1) / itemsPerPage);

		ImGui::SameLine();
		if (isLastPage)
			ImGui::BeginDisabled();
		if (ImGui::Button(u8"下一页"))
		{
			currentPage++;
		}
		if (isLastPage)
			ImGui::EndDisabled();

		ImGui::SameLine();
		if (isLastPage)
			ImGui::BeginDisabled();
		if (ImGui::Button(u8"最后一页"))
		{
			currentPage = sg_roomInfos.Size / itemsPerPage;
		}
		if (isLastPage)
			ImGui::EndDisabled();
	}
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::EndChild();


	if (GameClient::GetInstance().GetIsUpdateRoomList())
	{
		UpdateAndSortRooms();
		GameClient::GetInstance().SetIsUpdateRoomList(false);
	}
}