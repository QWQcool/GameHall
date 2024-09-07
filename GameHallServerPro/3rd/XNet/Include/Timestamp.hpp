#ifndef _TIMESTAMP_H_2024_4_12_
#define _TIMESTAMP_H_2024_4_12_

#include <string>
#include <chrono>
namespace X {

    class Timestamp
    {

    private:
        std::chrono::time_point<std::chrono::steady_clock> _oTime;

    public:
        Timestamp()
        {
            _oTime = std::chrono::steady_clock::now();
        }

        void Update()
        {
            _oTime = std::chrono::steady_clock::now();
        }
        //微妙
        uint64_t GetMicroSecond()
        {
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - _oTime).count();
        }
        //毫秒
        uint64_t GetMilliSecond()
        {
            return GetMicroSecond() / 1000;
        }

        static uint64_t GetNowMilliSecond()
        {
            return  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }

        static std::string GetNowInString(const char* timeFmt = "%d-%d-%d %d:%d:%02d")
        {
            // 时间输出格式查看  https://www.runoob.com/cprogramming/c-function-strftime.html
            // https://www.cnblogs.com/qicosmos/p/3642712.html
            // %Y %m %d %H:%M
            auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            struct tm t;   //tm结构指针
            char buff[1024];
#ifdef _WIN32
            localtime_s(&t, &time);
#else
            t = *(localtime(&time));
#endif
            strftime(buff, 1024, timeFmt, &t);
            return std::string(buff);
        }

    };
}
#endif
