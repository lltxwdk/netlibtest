#pragma once 

#include "TcpNetInterface.h"
#include "TcpListenSocket.h"
#include "EventLoop.h"

class TcpListenner :public ITcpListenner, public TcpListenSocket::ListennerSink
{
public:
	class TcpConnCreator
	{
	public:
		virtual void CreatNewConn(int sock_fd, char* remote_ip, uint16_t remote_port, ITcpListenEvent* ent_handler) = 0;
	};

public:
	TcpListenner(EventLoop* ent_loop_ins, ITcpListenEvent* ent_handler, TcpConnCreator& tcp_creator);
	virtual ~TcpListenner();
private:
	virtual bool Listen(const char* listen_ip, unsigned short listen_port, bool ipv6_mode = false);
	virtual void Close();
	virtual void Release();
private:
	virtual void OnAccept(int sock_fd, char* remote_ip, uint16_t remote_port);
	virtual void OnNetError(int err_code);
private:
	EventLoop* ent_loop_ins_;
	ITcpListenEvent* ent_handler_;
	TcpConnCreator& tcp_creator_;
	TcpListenSocket listen_sock_;

};