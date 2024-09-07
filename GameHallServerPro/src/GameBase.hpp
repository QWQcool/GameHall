#pragma once
#include<iostream>
#include<vector>
class GameBase
{
private:
	std::string _gameType;

public:
	virtual void Init() = 0;
	virtual ~GameBase() {}
};

class GoBang : public GameBase
{
private:
	int _boardWidth;
	int _boardHeight;
	int _playerIndex; //1是黑棋下，2是白旗下
	//x 15 y 15
	std::vector<std::vector<int>> _boardPiece;//存储落子位置 0 是空 1是黑子，2是白子
	std::pair<int, int> _lastPos;

	int _blackScore;
	int _whiteScore;

	int _continueGameState;
public:
	GoBang()
	{
		_boardWidth = 15;
		_boardHeight = 15;
		_playerIndex = 1;
		Init();
	}

	void SetDown(int x, int y, int playerIndex)
	{
		//和客户端一样的存储逻辑
		_boardPiece[y][x] = playerIndex;
		if (playerIndex == 1)_playerIndex = 2;
		if (playerIndex == 2)_playerIndex = 1;
		_lastPos.first = y;
		_lastPos.second = x;
	}

	void SetLastPos(int x, int y)
	{
		_lastPos.first = y;
		_lastPos.second = x;
	}

	void SetContinueGameState(int state)
	{
		_continueGameState = state;
	}

	int GetPlayerIndex() 
	{ 
		return _playerIndex; 
	}

	std::pair<int, int> GetLastPos() 
	{
		return _lastPos; 
	}

	std::vector<std::vector<int>> GetBoardPiece() const
	{
		return _boardPiece;
	}

	int GetBlackScore() 
	{
		return _blackScore; 
	}

	int GetWhiteScore() 
	{ 
		return _whiteScore; 
	}

	int GetContinueGameState() 
	{ 
		return _continueGameState; 
	}

	virtual void Init() override
	{
		//和客户端一样的多一逻辑
		_playerIndex = 1;
		_continueGameState = 0;
		_boardPiece.clear();
		_boardPiece.resize(_boardHeight + 1, std::vector<int>(_boardWidth + 1, 0)); // 初始化棋盘
		_lastPos.first = -1;
		_lastPos.second = -1;
	}

	bool CheckForWin(int x, int y, int playerIndex)
	{
		if (x < 0 || x >= _boardPiece.size() || y < 0 || y >= _boardPiece[0].size())
			return false;

		if (CheckWinCondition(playerIndex, x, y))
		{
			std::cout << "Player " << playerIndex << " wins!" << std::endl;
			if (playerIndex == 1)
				_blackScore++;
			else if (playerIndex == 2)
				_whiteScore++;
			return true;
		}
		return false;
	}

	bool CheckWinCondition(int playerIndex, int x, int y)
	{
		// 检查四个方向：水平、垂直、两个对角线
		const int directions[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };

		for (const auto& dir : directions)
		{
			int dx = dir[0], dy = dir[1];
			if (CountConsecutive(x, y, dx, dy, playerIndex) >= 5)
				return true;
		}

		return false;
	}

	int CountConsecutive(int x, int y, int dx, int dy, int playerIndex)
	{
		int count = 1; // 当前位置的棋子已经计算在内
		int nx = x + dx, ny = y + dy;

		// 向正方向检查
		while (nx >= 0 && nx < _boardWidth && ny >= 0 && ny < _boardHeight && _boardPiece[ny][nx] == playerIndex)
		{
			count++;
			nx += dx;
			ny += dy;
		}

		// 向反方向检查
		nx = x - dx;
		ny = y - dy;
		while (nx >= 0 && nx < _boardWidth && ny >= 0 && ny < _boardHeight && _boardPiece[ny][nx] == playerIndex)
		{
			count++;
			nx -= dx;
			ny -= dy;
		}

		return count;
	}

};

class GameFactory //游戏工厂
{
public:
	static GameBase* CreateGame(const std::string& gameType)
	{
		if (gameType == u8"五子棋")
			return new GoBang();
		else
			return nullptr; // 或者抛出异常
	}
};