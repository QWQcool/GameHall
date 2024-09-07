#pragma once
#include<iostream>
class Player
{
private:
	int _userId=-1;
	int _pos=-1;
	std::string _nickname;
	bool _isReady = false;

public:
	// Constructor
	Player(int userId, int pos, const std::string& nickname)
		: _userId(userId), _pos(pos), _nickname(nickname) {}

	Player(int userId, int pos, const std::string& nickname,bool isReady)
		: _userId(userId), _pos(pos), _nickname(nickname),_isReady(isReady) {}

	Player(int userId, const std::string& nickname)
		: _userId(userId), _nickname(nickname) {}
	// Setters
	void SetUserId(int userId)
	{
		_userId = userId;
	}

	void SetPosition(int pos)
	{
		_pos = pos;
	}

	void SetNickname(const std::string& nickname)
	{
		_nickname = nickname;
	}

	void SetReady()
	{
		_isReady = true;
	}

	void SetUnReady()
	{
		_isReady = false;
	}

	// Getters
	int GetUserId() const
	{
		return _userId;
	}

	int GetPosition() const
	{
		return _pos;
	}

	std::string GetNickname() const
	{
		return _nickname;
	}

	bool GetIsReady() const
	{
		return _isReady;
	}
};