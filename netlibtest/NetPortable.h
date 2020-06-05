#ifndef __NET_PORTABLE_H__
#define __NET_PORTABLE_H__

#include <string>
#include <sstream>
#include <set>
#include <vector>
#include <math.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment (lib, "ws2_32.lib")
#pragma comment( lib,"winmm.lib" )
#endif

#include "TypeDef.h"
#include "Utils.h"

#ifdef _WIN_XP_
const char* win32_inet_ntop(int af, const void *src, char* dst, socklen_t size);
int win32_inet_pton(int af, const char* src, void* dst);
#  define inet_ntop win32_inet_ntop
#  define inet_pton win32_inet_pton
#endif

#define IPV42STR(INADDR, IPSTR) \
char IPSTR[32] = { 0 };\
inet_ntop(AF_INET, &INADDR, IPSTR, 32);

inline int ConvertIPV4ToInt(const char* ip)
{
	struct in_addr in;
	int ret;
	ret = inet_pton(AF_INET, ip, &in);
	if (ret > 0)
	{
		return in.s_addr;
	}
	else
	{
		return 0;
	}

}
inline void ConvrtIPV4ToStr(int ip_be, char ipstr[32])
{
	struct in_addr in;
	in.s_addr = ip_be;
	inet_ntop(AF_INET, &in, ipstr, 32);
}

#ifdef _WIN32
typedef int socklen_t;
#define close_socket(x) if (x != INVALID_SOCKET) {::closesocket(x); x = INVALID_SOCKET;}
#else
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR            (-1)
#define close_socket(x) if (x != INVALID_SOCKET) {::close(x); x = INVALID_SOCKET;}
#endif


static bool getSourcePort0(int socket, uint16_t& resultPortNum)
{
	struct sockaddr_storage test;
	memset(&test, 0, sizeof(test));
	socklen_t len = sizeof(test);

	if (getsockname(socket, (struct sockaddr*)&test, &len) < 0)
	{
		return false;
	}

	if (test.ss_family == AF_INET6)
	{
		resultPortNum = ntohs(((struct sockaddr_in6 *)&test)->sin6_port);
	}
	else
	{
		resultPortNum = ntohs(((struct sockaddr_in *)&test)->sin_port);
	}

	return true;
}

static int GetErrCode()
{
#ifdef WIN32
	return GetLastError();
#else
	return errno;
#endif
}
static bool IsErrorEAGAIN(int nErrorCode)
{
#ifdef WIN32
	return(WSAEWOULDBLOCK == nErrorCode);
#else
	return EAGAIN == errno || EWOULDBLOCK == errno;
#endif
}

static bool IsErrorEINTR(int nErrorCode)
{
#ifdef WIN32
	return(WSAEINTR == nErrorCode);
#else
	return EINTR == errno;
#endif
}

static bool IsErrorERESET(int nErrorCode)
{
#ifdef WIN32
	return WSAECONNRESET == nErrorCode;
#else
	return ECONNRESET == nErrorCode;
#endif
}

static bool IsIgnoreSocketError(int &nErrorCode)
{
#ifdef WIN32
	nErrorCode = WSAGetLastError();
	if (WSAETIMEDOUT == nErrorCode
		|| WSAEWOULDBLOCK == nErrorCode
		|| WSAEINPROGRESS == nErrorCode
		|| WSAEINTR == nErrorCode) {
		return true;
	}
#else
	nErrorCode = errno;
	if (EAGAIN == errno
		|| EINTR == errno
		|| EINPROGRESS == errno
		|| EWOULDBLOCK == errno) {
		return true;
	}
#endif
	return false;
}

static bool IsIgnoreSocketError_UDP(int &nErrorCode)
{
	if (IsIgnoreSocketError(nErrorCode))
	{
		return true;
	}
#ifdef WIN32
	if (WSAECONNRESET == nErrorCode)
	{
		return true;
	}
#else
	if (ECONNRESET == nErrorCode)
	{
		return true;
	}
#endif
	return false;
}


#endif //_NET_COMMON_H_


