#ifndef _NETHEADER_H_2024_4_9_
#define _NETHEADER_H_2024_4_9_

#ifdef _WIN32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include<mswsock.h>
#pragma comment(lib,"ws2_32.lib")

// 高精度定时器库
// 导入定时器精度
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// #define X_NET_IOCP
#define X_NET_SELECT
#elif defined(__unix__) || defined(__linux__) // linux平台
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <csignal>
#include <fcntl.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKADDR_IN sockaddr_in
#define GetLastError() errno
#define X_NET_SELECT
#else
#define X_NET_SELECT
#endif

#include <iostream>
#include <vector>
#include <string>
#include <mutex>
#include <map>
#include <unordered_map>
#include <list>
#include <queue>
#include <set>
#include <functional>
#include <memory>
#include "Logger.hpp"

#include <iostream>
#include <cstdint>


namespace X
{
    class Socket
    {
    public:
        static void Close(SOCKET sock)
        {
#ifdef _WIN32
            closesocket(sock);
#else
            close(sock);

#endif
        }

        // 设置非阻塞
        static bool SetNonBlock(SOCKET fd)
        {
#ifdef _WIN32
            unsigned long ul = 1;
            int ret = ioctlsocket(fd, FIONBIO, (unsigned long *) &ul); //设置成非阻塞模式。
            return ret != SOCKET_ERROR;
#else
            return fcntl((fd), F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) == 0;
#endif
        }

        // 设置小包不合并
        static bool SetNoDelay(SOCKET fd)
        {
#ifdef _WIN32
            int bTrue = true ? 1 : 0;
            return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &bTrue, sizeof(bTrue)) == 0;
#else
            int bTrue = true ? 1 : 0;
            //return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&bTrue, sizeof(bTrue)) == 0;
            return true;
#endif
        }

        static void InitNetWork()
        {
#ifdef _WIN32
            //启动Windows socket 2.x环境
            WORD ver = MAKEWORD(2, 2);
            WSADATA dat;
            WSAStartup(ver, &dat);

            // 设置定时器精度
            timeBeginPeriod(1);
#endif
#ifndef _WIN32
            //if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
            //	return (1);
            //忽略异常信号，默认情况会导致进程终止
            signal(SIGPIPE, SIG_IGN);
#endif
        };
    };
}


#endif
