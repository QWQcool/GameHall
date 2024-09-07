#define _CRT_SECURE_NO_WARNINGS
#include "GoBangGame.h"
#include "GameClient.h"
#include "PlayerClient.h"
#include "ImguiAll.h"

static void* sg_GoBangBg = nullptr;
static void* sg_ReturnRoomBg = nullptr;

static void ImageInit()
{
	static bool init = false;
	if (init)
	{
		return;
	}
	init = true;
	sg_GoBangBg = LoadImageEx("./src/Image/GoBangImg.jpg");
	sg_ReturnRoomBg = LoadImageEx("./src/Image/ReturnRoomImg.jpg");
}

void GoBang::Init()
{
	_playerIndex = 1;
	_gameState = 1;
	_continueGameState = 0;
	//重置继续游戏按钮状态
	_playerClickedContinue = true;

	srand(time(NULL));
	_endDownTime = GetTickCount64()+60000;//当前时间+60秒

	//比初始的宽高略大一个，为了解决一些微妙的边界问题
	_boardPiece.clear();
	_boardPiece.resize(_boardHeight+1, std::vector<int>(_boardWidth+1 , 0)); // 初始化棋盘
	ImageInit();
}

void GoBang::ShowWidget()
{
	switch (_gameState)
	{
	case 1:
		ShowGameWidget();
		break;
	case 2:
		//预留给结束游戏菜单界面
		ShowEndWidget();
		break;
	default:
		break;
	}
}

void GoBang::TimerSendDownRequest()
{
	if (_endDownTime > GetTickCount64())return;
	//最多60以内找到随机位置
	long long clientUid = GameClient::GetInstance().GetPlayer()->GetUserId();
	int blackPID = _blackPlayer->GetUserId();
	int whitePID = _whitePlayer->GetUserId();
	bool blackDown = clientUid == blackPID && _playerIndex == 1;
	bool whiteDown = clientUid == whitePID && _playerIndex == 2;
	bool couldDown = blackDown||whiteDown;

	if (!couldDown)return;

	for (int i = 0; i < 60; i++)
	{
		//伪随机下棋
		int x = rand() % _boardWidth;
		int y = rand() % _boardHeight;
		if (_boardPiece[y][x] == 0)
		{
			//发送下棋请求
			GameClient::GetInstance().SendGoBangDownRequest(x, y, _playerIndex);
			ResetTimerEndDown();
			break;
		}
	}
}

void GoBang::ResetTimerEndDown()
{
	_endDownTime = GetTickCount64() + 60000;
}

void GoBang::ShowGameWidget()
{
	ImGui::Begin(u8"五子棋游戏窗口", nullptr);

	ImVec2 offset = ImGui::GetWindowPos();
	ImVec2 window_size = ImGui::GetWindowSize();
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 mousePos = ImGui::GetMousePos();

	// 添加一个变量来存储选定的颜色值
	// 使用ColorEdit3让用户选择颜色
	static float GameBgcolor[3] = { 0.85f, 0.78f, 0.58f }; // 初始化


	ImGui::SetCursorPos(ImVec2(0,0));
	ImGui::Image(sg_GoBangBg, ImVec2(window_size.x, window_size.y));

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (!window->SkipItems)
	{

		//画棋盘
		GoBang::DrawGoBoard(GameBgcolor);

		//输入检测
		GoBang::HandlePlayerInput();
		
		// 右侧：用户状态UI
		DrawUserStatus();

	}
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));//浅灰字
	// 调整ColorEdit3的位置
	ImVec2 colorPickerPos = ImVec2(window_size.x - 200, window_size.y - 100); // 示例位置
	ImGui::SetCursorPos(colorPickerPos);
	ImGui::ColorEdit3(u8"Board Color", GameBgcolor, ImGuiColorEditFlags_NoInputs);
	//获取鼠标位置
	if (ImGui::IsMousePosValid())
	{
		ImGui::SetCursorPos(ImVec2(window_size.x - 200, window_size.y - 75));
		ImGui::Text(u8"Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
		ImGui::Text(u8"Offset: (%g, %g)", offset.x, offset.y);
		if (mousePos.x >= offset.x + _offsetBoard.x + _margin / 2 && mousePos.x <= offset.x + _offsetBoard.x + _margin * 1.5 + (_boardWidth * _squareSize) &&
			mousePos.y >= offset.y + _offsetBoard.y + _margin / 2 && mousePos.y <= offset.y + _offsetBoard.y + _margin * 1.5 + (_boardHeight * _squareSize))
		{
			ImGui::SameLine();
			ImGui::Text(u8"Board pos: (%g, %g)", io.MousePos.x - offset.x - _offsetBoard.x - _margin, io.MousePos.y - offset.y - _offsetBoard.y - _margin);
		}
	}
	ImGui::PopStyleColor();
	ImGui::End();

}

void GoBang::DrawUserStatus()
{
	// 绘制用户状态
	ImVec2 windowSize = ImGui::GetWindowSize();
	std::string strBlackPlayer;
	std::string strWhitePlayer;

	if (_blackPlayer)
	{
		strBlackPlayer = _blackPlayer->GetNickname();
		if (_blackPlayer->GetUserId() == GameClient::GetInstance().GetId())
		{
			strBlackPlayer.append(u8"(自己)");
		}
	}

	if (_whitePlayer)
	{
		strWhitePlayer = _whitePlayer->GetNickname();
		if (_whitePlayer->GetUserId() == GameClient::GetInstance().GetId())
		{
			strWhitePlayer.append(u8"(自己)");
		}
	}
	ImGui::SetCursorPos(ImVec2(windowSize.x*0.55f, windowSize.y*0.3f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.0f, 0.0f, 0.5f));  // 黑底
	ImGui::BeginChild("##NowPlayer", ImVec2(windowSize.x * 0.18f, windowSize.y * 0.1f), true);  // 创建子窗口
	ImGui::PopStyleColor();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9765f, 0.1961f, 0.0471f, 1.0f));//橙红色
	if (_playerIndex == 1)
	{
		ImGui::Text(u8"当前下棋手: 黑棋选手"); //当前执棋手
	}
	else if (_playerIndex == 2)
	{
		ImGui::Text(u8"当前下棋手: 白棋选手");
	}
	//TODO 下棋时间定时器
	ImGui::Text(u8"%d秒后随机下棋", (int)((_endDownTime-GetTickCount64())/1000));
	ImGui::PopStyleColor();
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.75f, windowSize.y * 0.3f));  // 设置位置
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));  // 黑底
	ImGui::BeginChild("##blackPlayer", ImVec2(windowSize.x * 0.18f, windowSize.y * 0.15f), true);  // 创建子窗口
	ImGui::PopStyleColor();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));//白字
	ImGui::Text(u8"黑棋手: %s", _blackPlayer ? strBlackPlayer.c_str() : "None");
	ImGui::Text(u8"得分: %d/3", _blackScore);
	ImGui::PopStyleColor();
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.75f, windowSize.y * 0.45f));  // 设置位置
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));  // 白底
	ImGui::BeginChild("##whitePlayer", ImVec2(windowSize.x * 0.18f, windowSize.y * 0.15f), true);  // 创建子窗口
	ImGui::PopStyleColor();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8510f, 0.8824f, 0.9098f, 1.0f));//浅灰字
	ImGui::Text(u8"白棋手: %s", _whitePlayer ? strWhitePlayer.c_str() : "None");
	ImGui::Text(u8"得分: %d/3", _whiteScore);
	ImGui::PopStyleColor();
	ImGui::EndChild();

}

void GoBang::DrawGoBoard(float* GameBgColor)
{
	// 获取当前窗口的前景绘图列表
	ImDrawList* draw_list = ImGui::GetForegroundDrawList();
	ImVec2 offset = ImGui::GetWindowPos();
	offset.y = offset.y + _offsetBoard.y;
	offset.x = offset.x + _offsetBoard.x;

	// 将背景颜色应用到整个棋盘区域
	ImVec2 boardSize = ImVec2(_boardHeight * _squareSize + 20, _boardWidth * _squareSize + 20); // 计算棋盘大小
	draw_list->AddRectFilled(ImVec2(offset.x, offset.y), ImVec2(offset.x + boardSize.x, offset.y + boardSize.y), IM_COL32(GameBgColor[0] * 255, GameBgColor[1] * 255, GameBgColor[2] * 255, 255));

	// 绘制棋盘线
	for (int i = 0; i <= _boardWidth; ++i)
	{
		//// 横线
		draw_list->AddLine(
			ImVec2(_margin + offset.x, _margin + i * _squareSize + offset.y),
			ImVec2(_boardHeight * _squareSize + _margin + offset.x, _margin + i * _squareSize + offset.y),
			IM_COL32(230, 130, 45, 255));

		// 在棋盘左边绘制数字 ,几条横线几个数字
		char buffer[16]; // 文本缓冲区，确保足够大的空间以避免溢出
		if (i < 10)
		{
			sprintf(buffer, "%d", i); // 将数字转换为字符串
		}
		else
		{
			sprintf(buffer, "%c", i + 65 - 10);
		}
		ImVec2 textPos = ImVec2(_margin + offset.x - 20, _margin + i * _squareSize + offset.y - 10); // 计算文本位置，使其位于线的顶部
		draw_list->AddText(textPos, IM_COL32(0, 0, 0, 255), buffer); // 使用黑色绘制文本
	}

	for (int i = 0; i <= _boardHeight; ++i)
	{
		// 竖线
		draw_list->AddLine(ImVec2(_margin + i * _squareSize + offset.x, _margin + offset.y),
			ImVec2(_margin + i * _squareSize + offset.x, _boardWidth * _squareSize + _margin + offset.y),
			IM_COL32(230, 130, 45, 255));

		char buffer[16];
		sprintf(buffer, "%c", i + 65);
		ImVec2 letterPos = ImVec2(offset.x + _margin + i * _squareSize, offset.y - 20);
		draw_list->AddText(letterPos, IM_COL32(0, 0, 0, 255), buffer);
	}

	//绘制棋子
	for (int i = 0; i < _boardPiece.size(); i++)
	{
		for (int j = 0; j < _boardPiece[0].size(); j++)
		{
			//因为二维数组的循环问题，所以数组里存储的XY是相反的
			if (_boardPiece[j][i] == 1) // 注意这里的顺序
			{
				// 黑色棋子
				ImVec2 blackSquarePosition = ImVec2(_margin + i * _squareSize + offset.x, _margin + j * _squareSize + offset.y);
				draw_list->AddCircle(blackSquarePosition, _squareSize / 2, IM_COL32(0, 0, 0, 255));
			}
			if (_boardPiece[j][i] == 2) // 注意这里的顺序
			{
				// 白色棋子
				ImVec2 whiteSquarePosition = ImVec2(_margin + i * _squareSize + offset.x, _margin + j * _squareSize + offset.y);
				draw_list->AddCircle(whiteSquarePosition, _squareSize / 2, IM_COL32(255, 255, 255, 255));
			}
		}
	}
	////// 假设我们在(2,2)位置放置一个黑子的棋子
	//ImVec2 blackSquarePosition = ImVec2(_margin + 2 * _squareSize + offset.x, _margin + 2 * _squareSize + offset.y);
	//draw_list->AddCircle(blackSquarePosition, _squareSize / 2, IM_COL32(0, 0, 0, 255));
}

void GoBang::HandlePlayerInput()
{
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 offset = ImGui::GetWindowPos();
	ImDrawList* draw_list = ImGui::GetForegroundDrawList();
	offset.y = offset.y + _offsetBoard.y;
	offset.x = offset.x + _offsetBoard.x;
	ImVec2 mousePos = ImGui::GetMousePos();

	//检测自己是否是执棋人,且是否是对应回合
	int clientUid = GameClient::GetInstance().GetPlayer()->GetUserId();
	int blackPID = _blackPlayer->GetUserId();
	int whitePID = _whitePlayer->GetUserId();
	bool blackDown = clientUid == blackPID && _playerIndex==1;
	bool whiteDown = clientUid == whitePID && _playerIndex==2;
	//第三个条件用于下棋是否是自己检测
	if (mousePos.x >= offset.x + _margin / 2 && mousePos.x <= offset.x + _margin * 1.5 + (_boardWidth * _squareSize) &&
		mousePos.y >= offset.y + _margin / 2 && mousePos.y <= offset.y + _margin * 1.5 + (_boardHeight * _squareSize) &&
		(blackDown||whiteDown)
		)
	{
		int col = static_cast<int>((mousePos.x - offset.x + _margin) / _squareSize);
		int row = static_cast<int>((mousePos.y - offset.y + _margin) / _squareSize);
		//检测鼠标所在位置，预测落点方框
		ImVec2 curPos(offset.x + col * _squareSize - _squareSize / 2 + _margin, offset.y + row * _squareSize - _squareSize / 2 + _margin);
		draw_list->AddRect(curPos, ImVec2(curPos.x + _squareSize, curPos.y + _squareSize), IM_COL32(255, 0, 0, 255));

		if (row >= 0 && row <= _boardHeight && col >= 0 && col <= _boardWidth && _boardPiece[row][col] == 0
			&& ImGui::IsMouseClicked(ImGuiMouseButton_Left)
			)
		{
			GameClient::GetInstance().SendGoBangDownRequest(col, row, _playerIndex);
		}
	}
}

void GoBang::ShowEndWidget()
{
	ImGui::Begin("GameEndWidget", nullptr,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

	ImVec2 offset = ImGui::GetWindowPos();
	ImGuiIO& io = ImGui::GetIO(); 
	ImVec2 windowSize = ImGui::GetWindowSize();

	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::Image(sg_ReturnRoomBg, ImVec2(windowSize.x, windowSize.y));

	int clientUid = GameClient::GetInstance().GetPlayer()->GetUserId();
	int blackPID = _blackPlayer->GetUserId();
	int whitePID = _whitePlayer->GetUserId();
	bool blackDown = clientUid == blackPID;
	bool whiteDown = clientUid == whitePID;
	bool isGamePlayer = whiteDown || blackDown;

	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.15f, windowSize.y * 0.2f));  // 设置位置
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.7255f, 0.9451f, 0.5f));  // 设置颜色
	ImGui::BeginChild("##ReturnRoomBox", ImVec2(windowSize.x * 0.7f, windowSize.y * 0.4f), true);  // 创建子窗口
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

	ImGui::Text(u8"该局游戏结束\n");
	ImGui::Text(u8"目前黑棋选手比分:%d/%d", _blackScore, 3);
	ImGui::Text(u8"目前白棋选手比分:%d/%d", _whiteScore, 3);
	ImGui::Text(u8"只有下棋选手都同意才可以继续下一把，直至三胜，若是有下棋选手返回房间则直接结束游戏");

	bool isGameOver = false;
	if (_blackScore == 3 || _whiteScore == 3)
	{
		isGameOver = true;
	}

	if (isGameOver)
	{
		if(_blackScore==3)
		ImGui::Text(u8"游戏结束:黑棋选手获胜");
	else
		ImGui::Text(u8"游戏结束:白棋选手获胜");

		//一个计算当前时间的操作，在三秒后发送返回房间的请求
		ImGui::Text(u8"游戏结束，请等待三秒后自动返回房间");

	}

	if (ImGui::Button(u8"返回房间"))
	{
		// 返回房间的逻辑
		// 暂定返回房间时删除当前游戏类
		int Operator = 0; //0表示观战者，1表示黑棋选手，2表示白棋选手
		if (blackDown)
		{
			Operator = 1;
		}
		else if (whiteDown)
		{
			Operator = 2;
		}
		GameClient::GetInstance().SendReturnRoomRequest(Operator);
	}

	ImGui::SameLine();
	static const char* continueGameState[] = { u8"继续游戏0/2", u8"继续游戏1/2",u8"继续游戏2/2(BUG了！)"};

	bool couldContinue = isGamePlayer && _playerClickedContinue;

	if (couldContinue)
	{
		if (ImGui::Button(continueGameState[_continueGameState]))
		{
			if (couldContinue)
			{
		//不可以再次点击这个按钮了
			_playerClickedContinue = false;
			GameClient::GetInstance().SendAgainGameRequest(_continueGameState);
			}
		}
	}
	else
	{
		if (!isGameOver)
		{
		ImGui::Text(continueGameState[_continueGameState]);
		}
		else
		{
		ImGui::Text("等待返回房间");
		}
	}

	ImGui::PopStyleColor();
	ImGui::EndChild();  // 结束子窗口

	//获取鼠标位置
	if (ImGui::IsMousePosValid())
	{
		ImGui::SetCursorPos(ImVec2(windowSize.x - 200, windowSize.y - 75));
		ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
		ImGui::Text("offset: (%g, %g)", offset.x, offset.y);
	}

	ImGui::End();
}
