#ifndef EVENTLOOP_H
#define EVENTLOOP_H


#include "NetHeader.hpp"
#include "Timer.hpp"

namespace X
{
    typedef struct
    {
        SOCKET sock;
        SOCKADDR_IN addr;
    } NetPoint;

    class EventLoop
    {
    private:
        std::function<void(void *ptr)> _cb;
        Timer _timer;
#ifdef X_NET_IOCP
    public:
        struct IO_Handle
        {
            //重叠体
            OVERLAPPED overlapped;
            //
            SOCKET sockfd;
            //数据缓冲区
            WSABUF wsabuff;
            std::function<void(DWORD)> cb;
        };

    private:
        HANDLE _completionPort = NULL;

    private:
        bool InitIocp();

    public:
        //关联IOCP与sockfd
        bool Reg(SOCKET sockfd);

        bool PostRecv(IO_Handle *pHandle);
        bool PostSend(IO_Handle* pHandle);
        bool PostAccept(SOCKET _sock, IO_Handle *pHandle, int af);

        bool PostClose(IO_Handle *pHandle);
        bool IocpEvent(int ms);
        static SOCKADDR *GetAcceptExAddrs(X::EventLoop::IO_Handle *pIO_DATA, int af);

#endif
#ifdef X_NET_SELECT

    public:
        struct IO_Handle
        {
            SOCKET sockfd;
            std::function<void()> readEvent;
            std::function<void()> writeEvent;
            std::function<bool()> needWrite;
            std::function<void()> closeEvent;
        };

    private:
        std::unordered_map<SOCKET, IO_Handle *> _events;
        std::list<IO_Handle *> _removeEvents;

        fd_set _readfds;
        fd_set _writefds;
        SOCKET _sockpair[2];

        std::list<void *> _postQueue;
        std::mutex _postQueueMutex;

    private:
        void InitSelect();
    public:
        void AddEvent(IO_Handle *handle);

        void RemoveEvent(IO_Handle *handle);
#endif

    public:
        bool Init(std::function<void(void *ptr)> cb)
        {
            _cb = cb;
#ifdef X_NET_IOCP
            InitIocp();
#endif

#ifdef X_NET_SELECT
            InitSelect();
#endif

            return true;
        }

        bool LoopOnce(int ms = 1);

        void Post(void *ptr);

        uint64_t AddTimer(int ms, std::function<bool()> cb)
        {
            return _timer.AddTimer(ms, cb);
        }

        void RemoveTimer(uint64_t id)
        {
            _timer.CancelTimer(id);
        }
    };

}

#endif //EVENTLOOP_H
