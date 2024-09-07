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
	int _playerIndex; //1�Ǻ����£�2�ǰ�����
	//x 15 y 15
	std::vector<std::vector<int>> _boardPiece;//�洢����λ�� 0 �ǿ� 1�Ǻ��ӣ�2�ǰ���
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
		//�Ϳͻ���һ���Ĵ洢�߼�
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
		//�Ϳͻ���һ���Ķ�һ�߼�
		_playerIndex = 1;
		_continueGameState = 0;
		_boardPiece.clear();
		_boardPiece.resize(_boardHeight + 1, std::vector<int>(_boardWidth + 1, 0)); // ��ʼ������
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
		// ����ĸ�����ˮƽ����ֱ�������Խ���
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
		int count = 1; // ��ǰλ�õ������Ѿ���������
		int nx = x + dx, ny = y + dy;

		// ����������
		while (nx >= 0 && nx < _boardWidth && ny >= 0 && ny < _boardHeight && _boardPiece[ny][nx] == playerIndex)
		{
			count++;
			nx += dx;
			ny += dy;
		}

		// �򷴷�����
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

class GameFactory //��Ϸ����
{
public:
	static GameBase* CreateGame(const std::string& gameType)
	{
		if (gameType == u8"������")
			return new GoBang();
		else
			return nullptr; // �����׳��쳣
	}
};