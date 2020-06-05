#pragma once
#include "Socket.h"
#include "EventHandle.h"

class TcpListenSocket : public AsyncSocket, public EventHandler
{

public:
	class ListennerSink
	{
	public:
		virtual void OnAccept(int sock_fd, char* remote_ip, uint16_t remote_port) = 0;
		virtual void OnNetError(int err_code) = 0;
	};

	TcpListenSocket(ListennerSink* listenner_sink);
	virtual ~TcpListenSocket();

	bool Listen(const char* listen_ip, uint16_t listen_port);
	void Close();
private:
	virtual int GetEventFd(){ return GetSocketFd(); }
	virtual bool IsNeedWriteEvent(){ return false; }
	virtual void HandleEvent(bool ev_err, bool ev_read, bool ev_write);
private:
	ListennerSink* listenner_sink_;
};