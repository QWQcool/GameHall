#pragma once
#include "Protocol.h"
#include "ImguiAll.h"
#include<iostream>
#include<vector>

class Player;

// 示例游戏类型
class GoBang : public GameBase
{
private:
	Player* _blackPlayer; //小黑子
	Player* _whitePlayer; //小白
	std::vector<std::vector<int>> _boardPiece;//存储落子位置 0 是空 1是黑子，2是白子
	float _squareSize = 30.0f; // 每个方格的大小
	float _margin = 10.0f;     // 棋盘周围的边距
	ImVec2 _offsetBoard; //和棋盘左上角的XY坐标差

	int _playerIndex = 1; //1是黑，2是白
	int _gameState; //游戏状态 1 是游戏界面 0是游戏结束界面
	//
	
	int _boardWidth;//棋盘宽
	int _boardHeight;//棋盘高

	int _blackScore;
	int _whiteScore;
	int _continueGameState;
	bool _playerClickedContinue = true;

	int _endDownTime;
public:
	GoBang() : GameBase()
	{
		_whitePlayer = nullptr;
		_blackPlayer = nullptr;
		_boardWidth = 15;
		_boardHeight = 15;
		_offsetBoard=ImVec2(20, 50);
		Init();
	}
	virtual void Init() override; //初始化函数，每次初始化重置棋盘状态
	virtual void ShowWidget() override;

	void TimerSendDownRequest();
	void ResetTimerEndDown();

	void ShowGameWidget(); //游戏场景切换
	void DrawUserStatus(); // 绘制用户状态
	void DrawGoBoard(float* GameBgColor);//绘制棋盘
	void HandlePlayerInput(); //检测输入
	void ShowEndWidget();

	void DownPlayer(int x,int y ,int playerIndex,int nextPlayerIndex)
	{
		//因为二维数组的循环问题，所以数组里存储的XY是相反的
		_boardPiece[y][x] = playerIndex;
		_playerIndex = nextPlayerIndex;
		//当游戏结束时，如果当前游戏类下棋手是黑棋手，则是白棋手胜利
	}

	void SetPlayerIndex(int index)
	{
		_playerIndex = index;
	}
	void SetBlackPlayer(Player* player)
	{
		_blackPlayer = player;
	}

	void SetWhitePlayer(Player* player)
	{
		_whitePlayer = player;
	}

	void SetScore(int blackScore, int whiteScore)
	{
		_blackScore = blackScore;
		_whiteScore = whiteScore;
	}
	void SetContinueGameState(int state)
	{
		_continueGameState = state;
	}

	void SetGameState(int state) { _gameState = state; }

	Player* GetBlackPlayer() { return _blackPlayer; }
	Player* GetWhitePlayer() { return _whitePlayer; }
	int GetPlayerIndex() { return _playerIndex; }
	bool GetGameState() { return _gameState; }
	int GetContinueGameState() { return _continueGameState; }

	virtual void ClearGame() override
	{
		_boardPiece.clear();
		//delete _blackPlayer;
		_blackPlayer = nullptr;
		//delete _whitePlayer;
		_whitePlayer = nullptr;
	}
};
