#include "RoomClient.h"
#include "GameClient.h"
#include "PlayerClient.h"
#include <string>

static ImVec2 g_SquareWH(140, 100); //用于决定用户列表的框大小

// 用于存储被选中的玩家索引
static int sg_selected_player_index = -1;

// 用于存储鼠标悬停的玩家索引
static int sg_hovered_player_index = -1;

static void* sg_roomBg = nullptr;

static void ImageInit()
{
	static bool init = false;
	if (init)
	{
		return;
	}
	init = true;
	sg_roomBg = LoadImageEx("./src/Image/RoomImg.jpg");
}

void Room::ShowWidget()
{
	ImageInit();
	if (this->_isGameShow==1)
	{
		_game->ShowWidget();
	}
	else
	{
		ShowRoomWidget();
	}
}

void Room::Draw_LineGraph(int pos,ImVec2 graphChange ,ImU32 color)
{
	ImDrawList* draw_list = ImGui::GetForegroundDrawList();
	ImVec2 offset = ImGui::GetWindowPos();
	draw_list->AddRect(
		ImVec2(offset.x + graphChange.x, offset.y + graphChange.y), // 起始位置
		ImVec2(offset.x + graphChange.x + g_SquareWH.x, offset.y + graphChange.y + g_SquareWH.y), // 结束位置
		color
	);
}

void Room::Draw_TextGraph(int pos,ImVec2 graphChange,Player* player)
{
	ImDrawList* draw_list = ImGui::GetForegroundDrawList();
	ImVec2 offset = ImGui::GetWindowPos();

	ImVec2 textPos = ImVec2(offset.x + graphChange.x, offset.y + graphChange.y);
	// 绘制文本
	std::string showText;
	showText.append(u8"位置:");
	showText.append(std::to_string(pos+1));
	showText.append(u8"\n名字: ");
	showText.append(player->GetNickname());
	if (player->GetUserId() == GameClient::GetInstance().GetPlayer()->GetUserId())
	{
		showText.append(u8"(自己)");//表示是自己
	}
	if (player->GetIsReady())
	{
		showText.append(u8"\n已准备");
	}
	else
	{
		showText.append(u8"\n未准备");
	}
	if (player->GetUserId() == _roomOwnerId)//房主判断
	{
		showText.append(u8"\n房主");
		draw_list->AddText(textPos, IM_COL32(255, 46, 196, 182), showText.c_str()); // 使用#2EC4B6色文字
	}
	else
	{
		draw_list->AddText(textPos, IM_COL32(255, 255, 255, 255), showText.c_str()); // 使用白色文字
	}
}

void Room::ShowRoomWidget()
{
	ImGui::Begin(u8"游戏房间",nullptr);

	ImVec2 mouse_pos = ImGui::GetMousePos();
	ImVec2 offset = ImGui::GetWindowPos();

	int selfId = GameClient::GetInstance().GetPlayer()->GetUserId();
	int selfPos = GameClient::GetInstance().GetPlayer()->GetPosition();
	ImVec2 windowSize = ImGui::GetWindowSize();

	ImGui::SetCursorPos(ImVec2(0,0));
	ImGui::Image(sg_roomBg, ImVec2(windowSize.x, windowSize.y));

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f,1.0f,1.0f,1.0f));
	// 展示房间名称
	ImGui::SetCursorPos(ImVec2(windowSize.x*0.05f, windowSize.y*0.05f));
	ImGui::Text(u8"房间ID:%d", _roomId);
	ImGui::SameLine();
	ImGui::Text(u8"房名: %s", _roomName.c_str());
	ImGui::SameLine();
	ImGui::Text(u8"游戏类型: %s", _roomGameType.c_str());
	ImGui::SameLine();
	ImGui::Text(u8"房间人数: %d", _playerNum);
	ImGui::SameLine();
	ImGui::Text(u8"最大人数: %d", _playerMaxNum);

	// 操作按钮
	ImGui::SeparatorText(u8"房间信息分割线");
	ImGui::PopStyleColor();

	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.08f, windowSize.y * 0.25f));  // 设置位置
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 0.75f, 0.5f));  // 设置颜色
	ImGui::BeginChild("##loginbox", ImVec2(windowSize.x * 0.12f, windowSize.y * 0.5f), true);  // 创建子窗口
	ImGui::PopStyleColor();
	if (ImGui::Button(u8"离开房间"))
	{
		GameClient::GetInstance().SendExitRoomRequest(_roomId);
	}

	if (ImGui::Button(u8"清除位置选择"))
	{
		sg_selected_player_index = -1;
		sg_hovered_player_index = -1;
	}

	if (ImGui::Button(u8"移动位置"))
	{
		//额外的逻辑点击移动按钮后，移动至选中位置
		if (sg_selected_player_index != -1)
		{
			int clientUserId = GameClient::GetInstance().GetPlayer()->GetUserId();
			int oldPos = GameClient::GetInstance().GetPlayer()->GetPosition();
			GameClient::GetInstance().SendMovePosRequest(_roomId, clientUserId, oldPos, sg_selected_player_index);
			sg_selected_player_index = -1;
			sg_hovered_player_index = -1;
		}
	}
	// 仅显示准备按钮给零号位置和一号位置的玩家
		if (selfPos == 0 || selfPos == 1)
		{
			bool isReady = (bool)GameClient::GetInstance().GetPlayer()->GetIsReady();
			if (ImGui::Button(isReady ? u8"取消准备" : u8"准备"))
			{
				//发送现在是准备状态，服务端会返回一个非准备状态给客户端
				GameClient::GetInstance().SendReadyPosRequest(_roomId, selfId, selfPos, isReady);
			}
		}

	// 仅房主且两个位置的玩家都准备好时才显示开始游戏按钮
	if (_players[0] != nullptr && _players[1] != nullptr)
	{
		bool canStart = _roomOwnerId == selfId &&
			_players[0]->GetIsReady() &&
			_players[1]->GetIsReady();
		if (ImGui::Button(canStart ? u8"开始游戏" : u8"等待开始游戏"))
		{
			if (canStart)
			{
				GameClient::GetInstance().SendGamePlayRequest(_roomId);
			}
		}
	}
	else
	{
		if (ImGui::Button(u8"等待开始游戏"))
		{
		}
	}
	ImGui::EndChild();  // 结束操作按钮子窗口
	// 记录鼠标状态
	bool mouse_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

	for (int i = 0; i < _playerMaxNum; ++i)
	{
		//g_SquareWH = 200, 100;
		// 每行两个位置，每列高度为 100，每行高度为 g_SquareWH.y + 20（假设20为间隔）
		int row = i / 2;
		int col = i % 2;
		ImVec2 posGraph(250 + col * (g_SquareWH.x + 20), 200 + row * (g_SquareWH.y + 20));
		Draw_LineGraph(i, posGraph);

		// 检查鼠标是否在这个矩形内
		if (ImGui::IsMousePosValid())
		{
			if (mouse_pos.x >= offset.x + posGraph.x && mouse_pos.x <= offset.x + posGraph.x + g_SquareWH.x &&
				mouse_pos.y >= offset.y + posGraph.y && mouse_pos.y <= offset.y + posGraph.y + g_SquareWH.y)
			{
				sg_hovered_player_index = i;
			}
		}

		// 如果鼠标点击并且鼠标在某个矩形内，则选中此矩形
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && sg_hovered_player_index == i && _players[i]==nullptr
			&& mouse_pos.x >= offset.x + posGraph.x && mouse_pos.x <= offset.x + posGraph.x + g_SquareWH.x
			&& mouse_pos.y >= offset.y + posGraph.y && mouse_pos.y <= offset.y + posGraph.y + g_SquareWH.y
			)
		{
			sg_selected_player_index = i;
		}

		// 根据是否选中和是否悬停改变颜色
		ImU32 Squarecolor = IM_COL32(255, 0, 0, 255);
		if (sg_hovered_player_index == i)
			Squarecolor = IM_COL32(0, 0, 255, 255);
		if (sg_selected_player_index == i)
			Squarecolor = IM_COL32(0, 255, 0, 255);

		// 根据是否选中改变颜色
		Draw_LineGraph(i, posGraph, Squarecolor);

		if (_players[i]!=nullptr)
		{
			ImVec2 TextPosGraph(posGraph.x + 10, posGraph.y);
			ImGui::PushID(i); // 防止ID冲突
			Draw_TextGraph(i, TextPosGraph, _players[i]);
			ImGui::PopID();
		}

	}
	ImGui::End();
}

//玩家加入房间和更新房间存在用户状态用
bool Room::JoinRoom(Player* player)
{
	if (_playerNum >= _playerMaxNum)
	{
		return false;
	}
	//不该有这个_playerNum++; 客户端的目前客户应该由服务端下发数据
	//_playerNum++;
	int pos = player->GetPosition();
	if (pos == -1) return false;

	_players[pos] = player;

	return true;
}

bool Room::ExitRoom(int exitUserId)
{
	for (int i = 0; i < _playerMaxNum; i++)
	{
		if (_players[i] && _players[i]->GetUserId() == exitUserId)
		{
			_players[i] = nullptr;
			if (exitUserId != GameClient::GetInstance().GetId())
			{
				delete _players[i];
			}
			return true;
			//人数减少由服务端返回相应数据
		}
	}
	return false;
}

bool Room::GameStart()
{
	if (_game == nullptr)
	{
	_game = GameFactory::CreateGame(_roomGameType);
		if (_game != nullptr && dynamic_cast<GoBang*>(_game))
		{
			static_cast<GoBang*>(_game)->SetBlackPlayer(_players[0]);
			static_cast<GoBang*>(_game)->SetWhitePlayer(_players[1]);
			
		}
	}
	return false;
}
