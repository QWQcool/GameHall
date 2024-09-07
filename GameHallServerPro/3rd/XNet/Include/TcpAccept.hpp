//
// Created by Dream on 2024/7/10.
//

#ifndef TCPACCEPT_H
#define TCPACCEPT_H
#include "EventLoop.hpp"

namespace X
{
    class TcpAccept
    {
    protected:
        EventLoop *_loop;
        std::function<void(const X::NetPoint *const netPoint)> _cb;
        SOCKET _sock;
#ifdef X_NET_IOCP
        EventLoop::IO_Handle _accept;
        char buf[1024] = {0};
#endif
#ifdef X_NET_SELECT
        EventLoop::IO_Handle _accpet;
#endif

    public:
        TcpAccept(EventLoop *loop, std::function<void(const X::NetPoint *const netPoint)> cb)
        {
            _loop = loop;
            _cb = cb;
        }

        virtual ~TcpAccept()
        {
        }

        bool Listen(const int port, const char *const ip = "0.0.0.0", bool bReuse = true)
        {
            if (_cb == nullptr)
            {
                printf("TCP Accept Callback is null\n");
                return false;
            }

            SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            if (sock == INVALID_SOCKET)
            {
                return false;
            }
            // 端口复用
            if (bReuse)
            {
                const int reuse = 1;
                setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(reuse));
            }

            sockaddr_in addr = {0};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip);

            if (bind(sock, (sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
            {
                return false;
            }

            if (listen(sock, 5) == SOCKET_ERROR)
            {
                return false;
            }
            _sock = sock;
#ifdef X_NET_IOCP
            _loop->Reg(_sock);
            _accept.cb = [this](DWORD len)
            {
                SOCKET sock = _accept.sockfd;
                SOCKADDR *addr = _loop->GetAcceptExAddrs(&(this->_accept),AF_INET);
                X::NetPoint point;
                point.sock = sock;
                this->_cb(&point);

                PostAccept();
            };
            PostAccept();
#endif
#ifdef X_NET_SELECT
            _accpet.sockfd = sock;
            _accpet.readEvent = [this,sock]()
            {
                sockaddr_in addr = {0};
#ifdef _WIN32
                int len = sizeof(addr);
#else
                socklen_t len = sizeof(addr);
#endif

                SOCKET sockC = ::accept(sock, (sockaddr *) &addr, &len);

                if (sockC == INVALID_SOCKET)return;

                LOG_TRACE << "accept ip:" << inet_ntoa(addr.sin_addr)
                    << " port:" << ntohs(addr.sin_port);

                // socket NoDelay
#ifdef _WIN32
                int nodelay = 1;
                setsockopt(sockC, IPPROTO_TCP, TCP_NODELAY,
                           (const char *) &nodelay, sizeof(nodelay));
#else

#endif

                NetPoint netPoint;
                netPoint.sock = sockC;
                memcpy(&netPoint.addr, &addr, sizeof(addr));
                if (_cb != nullptr)
                    _cb(&netPoint);
                else
                    Socket::Close(sockC);
            };
            _loop->AddEvent(&_accpet);
#endif
            return true;
        }

    private:
#ifdef X_NET_IOCP
        void PostAccept()
        {
            ZeroMemory(&_accept.overlapped, sizeof(OVERLAPPED));
            _accept.wsabuff.buf = buf;
            _accept.wsabuff.len = 1204;
            _loop->PostAccept(_sock, &_accept,AF_INET);
        }
#endif
        void DoAccept();
    };
}

#endif //TCPACCEPT_H
