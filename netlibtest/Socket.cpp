#include "Socket.h"
#include "TraceLog.h"
#include <string.h>

AsyncSocket::AsyncSocket() :sock_fd_(), is_ipv6_()
{
	//构造函数初始化列表,C++ 初始化类成员时，是按照声明的顺序初始化的，而不是按照出现在初始化列表中的顺序。
}

AsyncSocket::~AsyncSocket()
{

}

void AsyncSocket::SetIpv6(bool ipv6_mode)
{
	is_ipv6_ = ipv6_mode;

	LogINFO("ip mode is:%s", (is_ipv6_ == true) ? "ipv6" : "ipv4");
}

bool AsyncSocket::CreateAsyncSocket(bool is_tcp)
{
	int type = is_tcp ? SOCK_STREAM : SOCK_DGRAM;
	int fd = is_ipv6_ ? socket(AF_INET6, type, 0) : socket(AF_INET, type, 0);

	if (fd < 0)
	{
		LogERR("Create socket fail! error code:%u, discrbie:%s", errno, strerror(errno));
		return false;
	}

	int buf_size = 500 * 1000;//500KB

	/*
		在send()的时候，返回的是实际发送出去的字节(同步)或发送到socket缓冲区的字节

		(异步);系统默认的状态发送和接收一次为8688字节(约为8.5K)；在实际的过程中发送数据

		和接收数据量比较大，可以设置socket缓冲区，而避免了send(),recv()不断的循环收发：
	*/
	if (0 != setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char *)&buf_size, sizeof(buf_size)))
	{
		TraceLog::GetInstance().TotalLog("netlib.log", "Socket:%d, setsockopt SO_RCVBUF error!, %d\n ",
			fd, GetErrCode());
	}
	if (0 != setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char *)&buf_size, sizeof(buf_size)))
	{
		TraceLog::GetInstance().TotalLog("netlib.log", "Socket:%d, setsockopt SO_SNDBUF error!, %d\n ",
			fd, GetErrCode());
	}

#if 1
	int optVal = 0;
	int optLen = sizeof(optVal);
	int ret = getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, (socklen_t*)&optLen);
	if (ret != 0)
	{
		TraceLog::GetInstance().TotalLog("netlib.log", "Socket:%d, getsockopt SO_RCVBUF error!, %d\n ",
			fd, GetErrCode());
	}
	else
	{
		printf("get socket rcvbuf len %d\n", optVal);
	}
#endif

	if (!SetNonblocking(fd))
	{
		LogERR("Create socket, SetNonblocking fail! error code:%u, discrbie:%s", errno, strerror(errno));
		close_socket(fd);
		return false;
	}

	if (is_ipv6_)
	{
		int on = 1;
		if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&on, sizeof(on)) < 0)
		{
			TraceLog::GetInstance().TotalLog("netlib.log", "Socket:%d, getsockopt SO_RCVBUF error!, %d\n ",
				fd, GetErrCode());
		}
	}

	if (is_tcp)
	{
		int val = 1;
		setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&val, sizeof(val));
	}

	sock_fd_ = fd;
	return true;

}

void AsyncSocket::CloseSocket()
{
	if (sock_fd_ != INVALID_SOCKET)
	{
		close_socket(sock_fd_);
		sock_fd_ = INVALID_SOCKET;
	}
}

bool AsyncSocket::Bind(const char* local_ip, uint16_t &local_port)
{
	struct sockaddr_storage local;
	memset(&local, 0, sizeof(local));

	if (is_ipv6_)
	{
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&local;
		sin6->sin6_family = AF_INET6;

		if (local_ip)
		{
			/*
			#include <arpe/inet.h>
			const char * inet_ntop(int family, const void *addrptr, char *strptr, size_t len);     //将数值格式转化为点分十进制的ip地址格式
			返回值：若成功则为指向结构的指针，若出错则为NULL
			int inet_pton(int family, const char *strptr, void *addrptr);     //将点分十进制的ip地址转化为用于网络传输的数值格式
			返回值：若成功则为1，若输入不是有效的表达式则为0，若出错则为-1
			
			*/
			inet_pton(AF_INET6, local_ip, &sin6->sin6_addr); // //将点分十进制的ip地址转化为用于网络传输的数值格式
		}
		else
		{
			sin6->sin6_addr = in6addr_any;
		}

		sin6->sin6_port = htons(local_port);
	}
	else
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)&local;
		sin->sin_family = AF_INET;
		sin->sin_addr.s_addr = local_ip ? inet_addr(local_ip) : 0;//htonl(local_ip);
		sin->sin_port = htons(local_port);
	}

	int ret = ::bind(sock_fd_, (struct sockaddr*)&local, sizeof(local));

	if (ret < 0)
	{
		LogERR("Bind socket (ip:%s,port:%u) fail! error code:%u, discrbie:%s", local_ip, local_port, errno, strerror(errno));
		//do nothing.close outside
		return false;
	}
	if (0 == local_port)
	{
		uint16_t port = 0;
		if (!getSourcePort0(sock_fd_, port))
		{
			LogERR("Bind socket (ip:%s,port:%u), getSourcePort0 fail! error code:%u, discrbie:%s", local_ip, local_port, errno, strerror(errno));
			//do nothing.close outside
			return false;
		}
		local_port = port;
	}
	return true;


}

bool AsyncSocket::SetNonblocking(int fd)
{
#ifdef _WIN32
	unsigned long imode = 1;
	int n = ioctlsocket(fd, FIONBIO, &imode);
	if (n != 0)
	{
		LogERR("errno = %d reason:%s %d\n", errno, strerror(errno), fd);
		return false;
	}
#else
	int get = fcntl(fd, F_GETFL);
	if (::fcntl(fd, F_SETFL, get | O_NONBLOCK) == -1)
	{
		LogERR("errno = %d reason:%s %d\n", errno, strerror(errno), fd);
		return false;
	}

	//recv timeout
	struct timeval timeout = { 3, 0 };//3s
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#endif

#ifdef IOS
	int set = 1;
	setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif

	return true;

}

bool AsyncSocket::ReuseAddr()
{
	int ret = 0;
	int reuse = 1;
#ifdef _WIN32
	ret = ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(int));
#else
	ret = ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
#endif
	if (ret < 0)
	{
		return false;
	}
	return true;
}

int AsyncSocket::GetSockError()
{
	int error = 0;
	socklen_t len = sizeof(error);
	if (getsockopt(sock_fd_, SOL_SOCKET, SO_ERROR, (char*)&error, &len) < 0)
	{
		//assert(false);
		return GetErrCode();
	}
	return error;
}

void AsyncSocket::SetIpv6(bool ipv6_mode)
{
	is_ipv6_ = ipv6_mode;

	LogINFO("ip mode is:%s", (is_ipv6_ == true) ? "ipv6" : "ipv4");

	return;
}

bool AsyncSocket::AttachAsyncSocket(int sock_fd)
{
	if (INVALID_SOCKET == sock_fd)
	{
		assert(false);
		return false;
	}

	if (!SetNonblocking(sock_fd))
	{
		close_socket(sock_fd);
		return false;
	}
	sock_fd_ = sock_fd;
	return true;
}