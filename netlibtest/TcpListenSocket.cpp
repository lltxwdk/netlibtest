#include "TcpListenSocket.h"

TcpListenSocket::TcpListenSocket(ListennerSink* listenner_sink)
{
	listenner_sink_ = listenner_sink;
}

TcpListenSocket::~TcpListenSocket()
{

}

bool TcpListenSocket::Listen(const char* listen_ip, uint16_t listen_port)
{
	if (IsValidSock())
	{
		return false;
	}
	if (!CreateAsyncSocket(true))
	{
		return false;
	}
	if (!ReuseAddr())
	{
		CloseSocket();
		return false;
	}
	if (!Bind(listen_ip, listen_port))
	{
		CloseSocket();
		return false;
	}

	if (::listen(GetSocketFd(), SOMAXCONN) < 0)
	{
		int err = GetSockError();
		CloseSocket();
		return false;
	}
	return true;

}

void TcpListenSocket::Close()
{
	CloseSocket();
}

void TcpListenSocket::HandleEvent(bool ev_err, bool ev_read, bool ev_write)
{
	if (ev_err)
	{
		int err_code = GetErrCode();
		if (listenner_sink_)
		{
			listenner_sink_->OnNetError(err_code);
		}
		return;
	}
	if (!ev_read)
	{
		return;
	}

	struct sockaddr_storage client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t size = sizeof(client_addr);

	int socket_client = ::accept(GetSocketFd(), (struct sockaddr*)(&client_addr), &size);

	if (socket_client < 0)
	{
		int err_code = GetErrCode();
		if (IsErrorEINTR(err_code))
		{
			return;
		}
		if (IsErrorEAGAIN(err_code))
		{
			return;
		}
		LogERR("accept error! reason is:%s", strerror(err_code));
		if (listenner_sink_)
		{
			listenner_sink_->OnNetError(err_code);
		}
		return;
	}

	if (listenner_sink_)
	{
		

		if (client_addr.ss_family == AF_INET6)
		{
			// ipv6
			LogINFO("OnAccept ipv6");
			char ip_str[128] = { 0 };
			inet_ntop(AF_INET6, &((struct sockaddr_in6 *)&client_addr)->sin6_addr, ip_str, 128);
			listenner_sink_->OnAccept(socket_client, ip_str, ntohs(((struct sockaddr_in6 *)&client_addr)->sin6_port));
		}
		else
		{
			LogINFO("OnAccept ipv4");

			IPV42STR((((struct sockaddr_in *)&client_addr)->sin_addr), ip_str);
			listenner_sink_->OnAccept(socket_client, ip_str, ntohs(((struct sockaddr_in *)&client_addr)->sin_port));
		}

	}
}