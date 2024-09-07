#ifndef _TIMER_H_2023_8_28_
#define _TIMER_H_2023_8_28_
#include "Timestamp.hpp"
#include <cassert>
#include <vector>
#include <functional>
#include <map>
namespace X
{
	class Timer
	{
	private:
		struct sTimer
		{
			std::function<bool()> cb;
			int delayms;
			uint64_t timerID;
			uint64_t expire;
		};
		//! 定时器
		std::map<uint64_t, sTimer*> _queTimer;
		std::map<uint64_t, sTimer*> _timerMap;
		std::vector<uint64_t> _timerRemove;
		uint64_t _timerID = 1;
		uint16_t _queSeq = 0; //! 用于生成定时器ID 会溢出
		volatile uint64_t _nextExpire; //! 最快触发时间

	private:
		void AddTimerEx(sTimer* time)
		{
			// 获取当前时间
			unsigned long long now = Timestamp::GetNowMilliSecond();

			// 计算之后的时间
			unsigned long long expire = now + time->delayms;
			assert(!(expire & 0xfffff00000000000));
			expire <<= 20;

			_queSeq++;
			// 添加一个尾部序列号
			expire |= (_queSeq & 0xfffff);

			time->expire = expire;

			_queTimer.insert(std::make_pair(expire, time));
			if (_nextExpire > now + time->delayms || _nextExpire < now)
			{
				_nextExpire = now + time->delayms;
			}
		}

	public:
		Timer()
		{
			_nextExpire = (unsigned long long) - 1;


		}
		uint64_t AddTimer(int delayms, std::function<bool()> cb)
		{
			// 获取当前时间
			unsigned long long now = Timestamp::GetNowMilliSecond();

			// 计算之后的时间
			unsigned long long expire = now + delayms;
			assert(!(expire & 0xfffff00000000000));
			expire <<= 20;

			_queSeq++;
			// 添加一个尾部序列号
			expire |= (_queSeq & 0xfffff);
			sTimer* t = new sTimer();
			t->cb = cb;
			t->delayms = delayms;
			uint64_t timerID = _timerID++;
			t->timerID = timerID;
			t->expire = expire;

			_timerMap.insert(std::make_pair(timerID, t));
			_queTimer.insert(std::make_pair(expire, t));
			if (_nextExpire > now + delayms || _nextExpire < now)
			{
				_nextExpire = now + delayms;
			}
			return timerID;
		}

		void UpdateTimer()
		{
			if (_queTimer.empty())return;

			unsigned long long nowMs = Timestamp::GetNowMilliSecond();
			unsigned long long expire = _nextExpire;
			if (expire <= nowMs)
			{
				std::vector<std::pair<uint64_t, sTimer*>> allexpire;

				while (1)
				{
					if (_queTimer.empty())
					{
						_nextExpire = (unsigned long long) - 1;
						break;
					}

					auto&& iter = _queTimer.begin();

					unsigned long long nextexpire = (iter->first) >> 20;
					if (nowMs < nextexpire)
					{
						_nextExpire = nextexpire;
						break;
					}
					allexpire.push_back(*iter);
					_queTimer.erase(iter);
				}

				for (auto iter = allexpire.begin(); iter != allexpire.end(); ++iter)
				{
					sTimer* time = iter->second;
					bool b = time->cb();
					if (b)
						AddTimerEx(time);
					else
					{
						_timerMap.erase(time->timerID);
						delete time;
					}
				}
			}

			do
			{
				if (_timerRemove.empty())break;
				auto begin = _timerRemove.begin();
				auto end = _timerRemove.end();
				for (; begin != end; ++begin)
				{
					uint64_t timerID = *begin;
					auto iter = _timerMap.find(timerID);

					if (iter == _timerMap.end()) continue;
					sTimer* time = iter->second;
					_queTimer.erase(time->expire);
					_timerMap.erase(iter);

					delete time;
				}
				_timerRemove.clear();

			} while (false);


		}

		bool CancelTimer(uint64_t timerID)
		{
			_timerRemove.push_back(timerID);
			return true;
		}

		uint64_t GetNextExpire() { return _nextExpire; }
		uint64_t GetNextExpireMs() { return _nextExpire == -1 ? 0 : _nextExpire - Timestamp::GetNowMilliSecond(); }
	};
}


#endif
