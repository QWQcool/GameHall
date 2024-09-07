#ifndef _TCPCLIENT_H_2024_1_3_
#define _TCPCLIENT_H_2024_1_3_
#define _CRT_SECURE_NO_WARNINGS
//#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
//#include <Winsock2.h>
#include <iostream>
#include <timeapi.h>
#pragma comment(lib,"ws2_32.lib")

class TcpClient
{
private:
    SOCKET _sockfd = INVALID_SOCKET;
    bool _isConnect = false;
    char _recvBuff[0x1000 * 100] = {};
    uint32_t _recvLen = 0;

    char _sendBuff[0x1000 * 100] = {};
    uint32_t _sendLen = 0;
public:

    //连接服务器
    int Connect(const char* ip, unsigned short port)
    {
        static bool bOne = false;
        if (!bOne)
        {
            bOne = true;
            WSAData wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            // 设置定时器精度
            //timeBeginPeriod(1);
            //停顿一毫秒
            Sleep(1);
        }

        _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        int ret = SOCKET_ERROR;

        // 2 连接服务器 connect
        sockaddr_in _sin = {};
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port);
#ifdef _WIN32
        _sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
        _sin.sin_addr.s_addr = inet_addr(ip);
#endif
        ret = connect(_sockfd, (sockaddr*)&_sin, sizeof(sockaddr_in));

        if (SOCKET_ERROR == ret)
        {
            printf("<socket=%d> connect <%s:%d> failed...", (int)_sockfd, ip, port);
        }
        else {
            _isConnect = true;
            OnConnect();
        }
        return ret;
    }

    bool IsConnect()
    {
        return _isConnect;
    }

    void LoopOnce()
    {
        if (!_isConnect)return;

        if (_sockfd == INVALID_SOCKET)return;

        FD_SET fdRead;
        FD_ZERO(&fdRead);

        FD_SET(_sockfd, &fdRead);

        timeval t = { 0,0 };
        int ret = select(_sockfd + 1, &fdRead, NULL, NULL, &t);

        if (ret < 0)return;

        if (FD_ISSET(_sockfd, &fdRead))
        {
            FD_CLR(_sockfd, &fdRead);

            int nLen = recv(_sockfd, _recvBuff + _recvLen, sizeof(_recvBuff) - _recvLen, 0);

            if (nLen <= 0)
            {
                printf("与服务器断开连接，任务结束\n");
                _isConnect = false;
                closesocket(_sockfd);
                _sockfd = INVALID_SOCKET;
                return;
            }

            _recvLen += nLen;

            while (_recvLen > 0)
            {

                int ret = OnNetMsg(_recvBuff, _recvLen);
                if (ret < 0)
                {
                    printf("处理网络消息失败，任务结束\n");
                    _isConnect = false;
                    closesocket(_sockfd);
                    return;
                }
                if (ret == 0)break;
                _recvLen -= ret;
                memcpy(_recvBuff, _recvBuff + ret, _recvLen);

            }

        }
    }

    void Send(const char* buf, int len)
    {
        send(_sockfd, buf, len, 0);
    }

public:

    virtual void OnConnect() = 0;

    virtual int OnNetMsg(const char* buf, int len) = 0;
};

#endif
