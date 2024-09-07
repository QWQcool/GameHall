#ifndef _TCPSOCKET_HPP_
#define _TCPSOCKET_HPP_
#include <cstring>

#include "EventLoop.hpp"

namespace X
{
    class TcpSocket
    {
#ifdef X_NET_IOCP

    protected:
        EventLoop::IO_Handle _recvHandle;
        EventLoop::IO_Handle _sendHandle;
        EventLoop::IO_Handle _closeHandle;
        bool _bSend = false;
#endif

#ifdef X_NET_SELECT

    protected:
        EventLoop::IO_Handle _handle;
        bool _bSend = false;
#endif

    protected:
        EventLoop *_loop;

        NetPoint _netPoint{};

        char *_recvBuffer;
        int _recvMaxSize; // 接收缓冲区大小
        int _recvSize; // 实际接收到的数据大小

        bool _bClose; // 是否关闭
        bool _bConnect; // 是否主动连接

        std::vector<char> _sendCache;
        std::vector<char> _sendData;

    public:
        explicit TcpSocket(const int recvMaxSize = 1024 * 8)
        {
            _recvMaxSize = recvMaxSize;
            _recvBuffer = new char[_recvMaxSize];
            _recvSize = 0;
//            _sendCache.reserve(_recvMaxSize * 2);

            memset(&_netPoint, 0, sizeof(_netPoint));

            _loop = nullptr;

            _bClose = false;
            _bConnect = false;

#ifdef X_NET_IOCP
            _recvHandle.sockfd = INVALID_SOCKET;
            _recvHandle.cb = std::bind(&TcpSocket::OnRecv, this, std::placeholders::_1);

            _sendHandle.sockfd = INVALID_SOCKET;
            _sendHandle.cb = std::bind(&TcpSocket::OnSend, this, std::placeholders::_1);

            _closeHandle.sockfd = INVALID_SOCKET;
            _closeHandle.cb = std::bind(&TcpSocket::OnClose, this, std::placeholders::_1);
#endif
#ifdef  X_NET_SELECT
            _handle.readEvent = [this]()
            {
                OnRecv();
            };
            _handle.needWrite = [this]()
            {
                return _sendData.size() || _sendCache.size();
            };
            _handle.writeEvent = [this]()
            {
                OnSend();
            };
            _handle.closeEvent = [this]()
            {
                OnClose();
            };
#endif
        }

        virtual ~TcpSocket()
        {
            delete[] _recvBuffer;
        }

        bool Connect(EventLoop *loop, const char *const ip, int port)
        {
            _loop = loop;
            SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            if (sock == INVALID_SOCKET)return false;

            sockaddr_in addr = {0};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip);

            //             // 设置为非阻塞
            // #if _WIN32
            //             unsigned long ul = 1;
            //             ioctlsocket(sock, FIONBIO, &ul);
            // #endif

            if (connect(sock, (sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
            {
                OnDisconnect();
                return false;
            } else
            {
                _bConnect = true;
                OnConnect();
#ifdef X_NET_IOCP
                _recvHandle.sockfd = _sendHandle.sockfd = sock;
                _loop->Reg(sock);
                PostRecv();
#endif
#ifdef X_NET_SELECT
                _handle.sockfd = sock;
                _loop->AddEvent(&_handle);
#endif

                return true;
            }
        }

        void OnAccept(const NetPoint *const netPoint, EventLoop *loop)
        {
            _loop = loop;

            _netPoint = *netPoint;
#ifdef X_NET_IOCP
            _recvHandle.sockfd = _sendHandle.sockfd = netPoint->sock;
            _loop->Reg(netPoint->sock);

            // int rcvbuf_size = 8192; // 设置接收缓冲区大小为 8192 字节
            // int sndbuf_size = 8192; // 设置发送缓冲区大小为 8192 字节
            //
            // // 设置接收缓冲区大小
            // setsockopt(_recvHandle.sockfd, SOL_SOCKET, SO_RCVBUF, (const char *) &rcvbuf_size, sizeof(rcvbuf_size));
            // // 设置发送缓冲区大小
            // setsockopt(_recvHandle.sockfd, SOL_SOCKET, SO_SNDBUF, (const char *) &sndbuf_size, sizeof(sndbuf_size));

            PostRecv();
#endif
            _handle.sockfd = _netPoint.sock;
            _loop->AddEvent(&_handle);
#ifdef X_NET_SELECT

#endif
        }

        void Send(const char *const buf, int len)
        {
            _sendCache.insert(_sendCache.end(), buf, buf + len);

            if (_sendCache.size() >= _recvMaxSize || _sendData.size() >= _recvMaxSize)
            {
                LOG_DEBUG << "_sendCache.size() " << _sendCache.size() << " _sendData.size()" << _sendData.size();
            }

#ifdef X_NET_IOCP
            PostSend();
#endif
        }

        void Close()
        {
            if (_bClose)
            {
                LOG_ERROR << "客户端重复关闭";
                return;
            }
            _bClose = true;
#ifdef X_NET_IOCP
            _loop->PostClose(&_closeHandle);
#endif
#ifdef X_NET_SELECT
            _loop->RemoveEvent(&_handle);
#endif
        }

    public:
        virtual void OnConnect()
        {
        }

        virtual int OnNetMsg(const char *const buff, int len) = 0;

        virtual void OnDisconnect() = 0;
#ifdef X_NET_SELECT

    private:
        void OnRecv()
        {
            int nRecv = recv(_handle.sockfd, _recvBuffer + _recvSize, _recvMaxSize - _recvSize, 0);

            if (nRecv <= 0)
            {
                Close();
                return;
            }

            _recvSize += nRecv;

            while (_recvSize > 0)
            {
                // <0 出错 需要关闭连接
                // =0 表示接收到的数据不完整
                // >0 表示接收到的数据完整 需要删除数据段

                int nRet = OnNetMsg(_recvBuffer, _recvSize);
                if (nRet > 0)
                {
                    _recvSize -= nRet;
                    if (_recvSize <= 0)
                    {
                        _recvSize = 0;
                        break;
                    } else
                        memmove(_recvBuffer, _recvBuffer + nRet, _recvSize);
                    if (_recvSize < 0)
                    {
                        printf("error--------------------\n");
                    }
                } else
                    break;
            }
        }

        void OnSend()
        {
            if (_sendData.size() == 0 && _sendCache.size())
                _sendData.swap(_sendCache);
            if (_sendData.size() == 0)return;
            int nSend = send(_handle.sockfd, &_sendData[0], _sendData.size(), 0);
            _sendData.erase(_sendData.begin(), _sendData.begin() + nSend);
        }

        void OnClose()
        {
            // LOG_DEBUG << "关闭客户端";
            Socket::Close(_handle.sockfd);
            OnDisconnect();
        }
#endif


#ifdef X_NET_IOCP

    protected:
        void PostRecv()
        {
            if (_bClose)return;
            _recvHandle.wsabuff.buf = _recvBuffer + _recvSize;
            _recvHandle.wsabuff.len = _recvMaxSize - _recvSize;
            if (_recvMaxSize - _recvSize <= 0)
            {
                LOG_ERROR << "数据超出为处理强制关闭 _recvMaxSize:" << _recvMaxSize << "  _recvSize:" << _recvSize;
                Close();
                return;
            }
            if (!_loop->PostRecv(&_recvHandle))
            {
                LOG_ERROR << "投递接收数据失败关闭客户端";
                Close();
            }
        }

        void OnRecv(DWORD len)
        {
            if (len <= 0)
            {
                Close();
                return;
            }
            _recvSize += len;
            // LOG_DEBUG << (int) len << "  " << _recvBuffer << "   ";

            while (_recvSize > 0)
            {
                // <0 出错 需要关闭连接
                // =0 表示接收到的数据不完整
                // >0 表示接收到的数据完整 需要删除数据段

                int nRet = OnNetMsg(_recvBuffer, _recvSize);
                if (nRet > 0)
                {
                    _recvSize -= nRet;
                    if (_recvSize <= 0)
                    {
                        _recvSize = 0;
                        break;
                    } else
                        memmove(_recvBuffer, _recvBuffer + nRet, _recvSize);
                    if (_recvSize < 0)
                    {
                        printf("error--------------------\n");
                    }
                } else
                    break;
            }

            PostRecv();
        }

        void PostSend()
        {
            if (_bSend || (_sendData.size() == 0 && _sendCache.size() == 0))return;
            _bSend = true;

            if (_sendData.size() == 0)
                _sendCache.swap(_sendData);

            _sendHandle.wsabuff.buf = &_sendData[0];
            _sendHandle.wsabuff.len = _sendData.size();

            _loop->PostSend(&_sendHandle);
        }

        void OnSend(DWORD nSend)
        {
            _bSend = false;
            if (nSend <= 0)
            {
                LOG_ERROR << "发送据失败:nSend:" << (int) nSend << " Err:" << (int) GetLastError();
                Close();
                return;
            }
            _sendData.erase(_sendData.begin(), _sendData.begin() + nSend);
            PostSend();
        }

        void OnClose(DWORD len)
        {
            // LOG_DEBUG << "关闭客户端";
            closesocket(_recvHandle.sockfd);
            OnDisconnect();
        }
#endif
    };
}
#endif
