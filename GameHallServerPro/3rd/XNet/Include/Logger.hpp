#ifndef _LOGGER_H_2024_4_12_
#define _LOGGER_H_2024_4_12_


#include <cstdint>
#include <list>
#include <mutex>
#include <iostream>
#include <chrono>
#include <cstring>
#include <thread>
#include <algorithm>

namespace X
{

	enum class LogLevel {
		ETRACE = 0,   // 用于流程跟踪
		EDEBUG,       // 用于输出调试信息
		EINFO,        // 用于输出反馈信息
		EWARN,        // 警告
		EERROR,       // 错误
		EFATAL,       // 严重错误
		ESYSERR,   // 系统错误
		ESYSFATAL, // 系统文件
		ENUM_LOG_LEVELS,
	};

	class ILogBuff
	{
	public:
		virtual ~ILogBuff() {}
		virtual void Release() = 0;
		virtual uint32_t Length() = 0;
		virtual const char* Data() = 0;
	};


	class LogBuff :public ILogBuff
	{
		char* _buff;
		uint32_t _nowLen;
		uint32_t _maxLen;
	public:
		explicit LogBuff(int len = 4096)
		{
			_buff = new char[len];
			_nowLen = 0;
			_maxLen = len;
		}
		~LogBuff() override { delete[] _buff; }

		void Release() override { delete this; }

		uint32_t Length() override { return _nowLen; }

		const char* Data() override { return _buff; }

		bool Append(const char* buf, uint32_t len)
		{
			if (!(Avail() > len))
				ApplyMem(len);

			memcpy(_buff + _nowLen, buf, len);
			_nowLen += len;
			return true;
		}

		// 剩余可用的长度
		uint32_t Avail() const { return static_cast<uint32_t>(_maxLen - _nowLen); }

		char* Current() { return _buff + _nowLen; }

		void Add(uint32_t nLen) { _nowLen += nLen; }

		void ApplyMem(uint32_t len)
		{
			if (_nowLen + len > _maxLen)
			{
				do {
					_maxLen = _maxLen * 2;
				} while (_nowLen + len > _maxLen);

				char* buf = new char[_maxLen];
				memcpy(buf, _buff, _nowLen);
				delete[] _buff;
				_buff = buf;
			}
		}

	};

	class ILogger {
	public:
		/**
		 * @描述 线程不安全 会在多个线程中调用
		 */
		virtual void Output(X::LogLevel level, X::ILogBuff* buff) = 0;
	};



	class Logger
	{
	private:
		LogBuff* _buff;
		X::LogLevel _eLevel;
		struct LoggerEX
		{
			X::ILogger* s_pILogger;
			X::LogLevel s_eLevel = X::LogLevel::ETRACE;
			LoggerEX(X::ILogger* pILogger_, X::LogLevel eLevel_) :s_pILogger(pILogger_), s_eLevel(eLevel_) {}
		};


		class Log :public X::ILogger
		{
		private:
			//moodycamel::ConcurrentQueue
			std::list<X::ILogBuff*> _queue;
			std::mutex _queueMutex;
		public:
			Log()
			{
				std::thread t(&Log::Run, this);
				t.detach();
			}
			// 通过 ILogger 继承
			virtual void Output(X::LogLevel level, X::ILogBuff* buff) override
			{
				//_queue.enqueue(buff);
				_queueMutex.lock();
				_queue.push_back(buff);
				_queueMutex.unlock();
			}
		private:
			void Run()
			{
				while (true)
				{
					X::ILogBuff* buff;

					if (_queue.empty())
					{
						//std::this_thread::sleep_for(std::chrono::milliseconds(1));
#if _WIN32
						// Sleep(1);
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
#else
#endif
						continue;
					}
					do
					{
						//std::list<X::ILogBuff*> caches;
						//caches.swap(_queue);
						_queueMutex.lock();
						buff = _queue.front();
						_queue.pop_front();
						_queueMutex.unlock();
						if (buff)
						{
							std::cout << std::string(buff->Data(), buff->Length()) << std::endl;
							buff->Release();
						}

					} while (false);
				}
			}
		};

	public:
		static struct LoggerEX* Ins()
		{
			static struct LoggerEX ins { nullptr, X::LogLevel::ETRACE };
			return &ins;
		}
		static void SetILogger(ILogger* pILogger_) { Ins()->s_pILogger = pILogger_; }

		static ILogger* GetILogger() { return Ins()->s_pILogger; }

		static LogLevel GetLevel() { return Ins()->s_eLevel; }

		static void SetLevel(LogLevel eLevel) { Ins()->s_eLevel = eLevel; }

	public:
		template< int N2>
		Logger(const char(&pFunc)[N2], int nLine, X::LogLevel level) {
			static const char* LogLevelName[(int)LogLevel::ENUM_LOG_LEVELS] =
			{
					"T",
					"D",
					"I",
					"W",
					"E",
					"F",
					"S",
					"SF",
			};
//            {
//					"TRACE   ",
//					"DEBUG   ",
//					"INFO    ",
//					"WARN    ",
//					"ERROR   ",
//					"FATAL   ",
//					"SYSERR  ",
//					"SYSFATAL",
//			};
			_buff = new LogBuff();
			_eLevel = level;
			// 时间
			char ch[64];
#ifdef _WIN32
			__time64_t long_time;
			struct tm t;
			_time64(&long_time);
			_localtime64_s(&t, &long_time);
			size_t nRet = sprintf_s(ch, 64, "%d-%d-%d %d:%d:%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
			//strftime(ch, sizeof(ch), "%Y-%m-%d %H-%M-%S", localtime(&t)); //年-月-日 时-分-秒
#else
			time_t t = time(0);
			size_t nRet = strftime(ch, sizeof(ch), "%Y-%m-%d %H-%M-%S", localtime(&t)); //年-月-日 时-分-秒
#endif
			_buff->Append("[", 1);
			_buff->Append(ch, (uint32_t)nRet);
			_buff->Append("]", 1);
			// 日志等级
			_buff->Append("[", 1);
			_buff->Append(LogLevelName[(int)level], 1);
			//*this << LogLevelName[(int)level];
			_buff->Append("]", 1);


			// 函数名
			_buff->Append("[", 1);
			_buff->Append(pFunc, N2 - 1);
			_buff->Append("]", 1);

			// 行号
			_buff->Append("[", 1);
			*this << nLine;
			_buff->Append("]:", 2);

		}

		~Logger()
		{
			_buff->Append("\0", 1);
			//std::cout << std::string(_buff->Data(), _buff->Length()) << std::endl;
			//_buff->Release();

			if (Ins()->s_pILogger == nullptr)
				Ins()->s_pILogger = new Log();
			Ins()->s_pILogger->Output(_eLevel, _buff);

		}

	public:
		Logger& operator<<(const char* pBuff)
		{
			if (pBuff)
				for (int i = 0; pBuff[i]; i++)
					_buff->Append(pBuff + i, 1);
			//_buff->Append(pBuff, (uint32_t)strlen(pBuff));
			else
				_buff->Append("(null)", 6);
			return *this;
		}
		Logger& operator<<(const std::string& str)
		{
			_buff->Append(str.c_str(), (uint32_t)str.length());
			return *this;
		}


		Logger& operator<<(int v) {
			FormatInteger(v);
			return *this;
		}

		Logger& operator<<(uint64_t v) {
			FormatInteger(v);
			return *this;
		}

		Logger& operator<<(const void* p) {
			uintptr_t v = reinterpret_cast<uintptr_t>(p);
			if (_buff->Avail() >= 32)
				_buff->ApplyMem(32);

			char* buf = _buff->Current();
			buf[0] = '0';
			buf[1] = 'x';
			size_t len = ConvertHex(buf + 2, v);
			_buff->Add((uint32_t)len + 2);
			return *this;

			return *this;
		}

	private:
		template<typename T>
		void FormatInteger(T v) {
			if (_buff->Avail() >= 32)
				_buff->ApplyMem(32);


			size_t len = Convert(_buff->Current(), v);
			_buff->Add((int)len);
		}

		template<typename T>
		size_t Convert(char* buf, T value) {
			static const char digits[] = "9876543210123456789";
			static const char* zero = digits + 9;
			T i = value;
			char* p = buf;
			do {
				int lsd = static_cast<int>(i % 10);
				i /= 10;
				*p++ = zero[lsd];
			} while (i != 0);

			if (value < 0) {
				*p++ = '-';
			}
			*p = '\0';
			std::reverse(buf, p);
			return p - buf;
		}

		size_t ConvertHex(char buf[], uintptr_t value) {
			uintptr_t i = value;
			char* p = buf;
			static const char digitsHex[] = "0123456789ABCDEF";
			do {
				int lsd = static_cast<int>(i % 16);
				i /= 16;
				*p++ = digitsHex[lsd];
			} while (i != 0);

			*p = '\0';
			std::reverse(buf, p);

			return p - buf;
		}

	};

#define LOG____(X1)  if(true)X::Logger(__FUNCTION__, __LINE__, X1)

	// 用于流程跟踪
#define LOG_TRACE                                      \
    if (X::Logger::GetLevel() <= X::LogLevel::ETRACE) \
    LOG____(X::LogLevel::ETRACE)


	// 用于输出调试信息
#define LOG_DEBUG                                      \
    if (X::Logger::GetLevel() <= X::LogLevel::EDEBUG) \
    LOG____(X::LogLevel::EDEBUG)

// 用于输出反馈信息
#define LOG_INFO                                      \
    if (X::Logger::GetLevel() <= X::LogLevel::EINFO) \
    LOG____(X::LogLevel::EINFO)

// 警告
#define LOG_WARN LOG____(X::LogLevel::EWARN)

// 错误
#define LOG_ERROR LOG____(X::LogLevel::EERROR)

// 严重错误
#define LOG_FATAL LOG____(X::LogLevel::EFATAL)

// 系统错误
#define LOG_SYSERR LOG____(X::LogLevel::ESYSERR)

// 系统文件
#define LOG_SYSFATAL LOG____(X::LogLevel::ESYSFATAL)

}


#endif
