#ifndef _NET_COMMON_H_
#define _NET_COMMON_H_

#include <string>
#include <sstream>
#include <set>
#include <vector>
#include <math.h>

#ifdef __GNUC__
#ifndef SR_UNUSED 
#define SR_UNUSED __attribute__((unused))
#endif
#else
#ifndef SR_UNUSED 
#define SR_UNUSED
#endif

#endif


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

//ip save as host byte order 
//host byte order: use it in program
//net byte order: use it when send data or use socket api
#ifdef _WIN_XP_
const char* inet_ntop_v4(const void* src, char* dst, socklen_t size) {
	if (size < INET_ADDRSTRLEN) {
		return nullptr;
	}
	const struct in_addr* as_in_addr =
		reinterpret_cast<const struct in_addr*>(src);
	/*rtc::sprintfn*/_snprintf(dst, size, "%d.%d.%d.%d",
		as_in_addr->S_un.S_un_b.s_b1,
		as_in_addr->S_un.S_un_b.s_b2,
		as_in_addr->S_un.S_un_b.s_b3,
		as_in_addr->S_un.S_un_b.s_b4);
	return dst;
}
const char* win32_inet_ntop(int af, const void *src,
	char* dst, socklen_t size) {
	if (!src || !dst) {
		return nullptr;
	}
	switch (af) {
	case AF_INET: {
		return inet_ntop_v4(src, dst, size);
	}
	case AF_INET6: {
		return NULL;
	}
	}
	return nullptr;
}
#endif
class InetAddress
{
public:
  InetAddress() :ip_(0), port_(0)
  {

  }
  InetAddress(const InetAddress &addr) : ip_(addr.ip_), port_(addr.port_)
  {

  }
  InetAddress(uint32_t ip, uint16_t port) : ip_(ip), port_(port)
  {

  }
  InetAddress(std::string strIpAddr, uint16_t nPort)
  {
    ip_ = ntohl(inet_addr(strIpAddr.c_str()));
    port_ = nPort;
  }
  std::string ToString()
  {
    std::string ip;
    std::string ipSec;
    std::stringstream ss;
    uint32_t leftValue = ip_;
    for (int i = 3; i >= 0; i--) {
      uint32_t temp = (uint32_t)pow(256.0, i);
      uint32_t sectionValue = leftValue / temp;
      leftValue %= temp;
      ss << sectionValue;
      ss >> ipSec;
      ss.clear();
      if (i != 0) {
        ipSec.append(".");
      }
      ip.append(ipSec);
      ipSec.clear();
    }

    return ip;
  }

  std::string AsString()
  {
    std::string strAddress = ToString();
    return Utils::String::AppendFormat(strAddress, ":%u", port_);
  }

  void Clear()
  {
    ip_ = 0;
    port_ = 0;
  }

  bool Empty()
  {
    return (ip_ == 0 || port_ == 0);
  }

  bool operator()(const InetAddress &nAddr1, const InetAddress &nAddr2) const
  {
    if (nAddr1.ip_ > nAddr2.ip_) {
      return true;
    } else if (nAddr1.ip_ == nAddr2.ip_) {
      return nAddr1.port_ > nAddr2.port_;
    } else {
      return false;
    }
  }

  bool operator == (const InetAddress nAddr)
  {
    if ((nAddr.ip_ == ip_) && (nAddr.port_ == port_)) {
      return true;
    }

    return false;
  }

  InetAddress& operator = (const InetAddress& nAddr)
  {
    ip_ = nAddr.ip_;
    port_ = nAddr.port_;
    return *this;
  }

  bool operator < (const InetAddress& nAddr) const
  {
    if (ip_ < nAddr.ip_) {
      return true;
    } else if (ip_ == nAddr.ip_) {
      return port_ < nAddr.port_;
    } else {
      return false;
    }
  }

#ifdef WEBRTC_MAC
  uint32_t ip() const {return ip_;}
#else
  const uint32_t ip() const {return ip_;}
#endif
#ifdef WEBRTC_MAC
  uint16_t port() const {return port_;}
#else
  const uint16_t port() const {return port_;}
#endif

//private:
  uint32_t ip_;
  uint16_t port_;
};

struct P2pIPAddress
{
    P2pIPAddress()
		:id(0)
		, nat_typ(0)
		, local_addr_count(0)
	{
	}
    uint32_t id;
    uint8_t nat_typ;
    uint8_t local_addr_count;
    InetAddress mapped_addr;
	InetAddress use_srun_addr;
    InetAddress local_addr[8];
};

typedef std::vector<InetAddress> InetAddressArray;
typedef std::set<InetAddress> InetAddressSet;

#define MAKE_SOCKADDR_IN(var,adr,prt) /*adr,prt must be in network order*/ \
	struct sockaddr_in var;\
	var.sin_family = AF_INET;\
	var.sin_addr.s_addr = (adr);\
	var.sin_port = (prt);

#ifdef _WIN32
typedef int socklen_t;
#define close_socket(x) if (x != INVALID_SOCKET) {::closesocket(x); x = INVALID_SOCKET;}
#else
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR            (-1)
#define close_socket(x) if (x != INVALID_SOCKET) {::close(x); x = INVALID_SOCKET;}
#endif

static bool getSourcePort0(int socket, uint16_t& resultPortNum) SR_UNUSED;
static bool getSourcePort0(int socket, uint16_t& resultPortNum) {
  sockaddr_in test; test.sin_port = 0;
  socklen_t len = sizeof test;
  if (getsockname(socket, (struct sockaddr*)&test, &len) < 0) return false;

  resultPortNum = ntohs(test.sin_port);
  return true;
}

static bool getSourcePort(int socket, uint32_t ip, uint16_t& port) SR_UNUSED;
static bool getSourcePort(int socket, uint32_t ip, uint16_t& port) {
  uint16_t portNum = 0;
  if (!getSourcePort0(socket, portNum) || portNum == 0) {
    // Hack - call bind(), then try again:
    MAKE_SOCKADDR_IN(name, htonl(ip), 0);
    ::bind(socket, (struct sockaddr*)&name, sizeof name);

    if (!getSourcePort0(socket, portNum) || portNum == 0) {
      return false;
    }
  }

  port = portNum;
  return true;
}

static int GetErrCode() SR_UNUSED;
static int GetErrCode()
{
#ifdef WIN32
    return GetLastError();
#else
    return errno;
#endif
}

static bool IsErrorEAGAIN(int nErrorCode) SR_UNUSED;
static bool IsErrorEAGAIN(int nErrorCode)
{
#ifdef WIN32
    return(WSAEWOULDBLOCK == nErrorCode);
#else
    return EAGAIN == errno || EWOULDBLOCK == errno;
#endif
}

static bool IsErrorEINTR(int nErrorCode) SR_UNUSED;
static bool IsErrorEINTR(int nErrorCode)
{
#ifdef WIN32
    return(WSAEINTR == nErrorCode);
#else
    return EINTR == errno;
#endif
}

static bool IsErrorERESET(int nErrorCode) SR_UNUSED;
static bool IsErrorERESET(int nErrorCode)
{
#ifdef WIN32
    return WSAECONNRESET == nErrorCode;
#else
    return ECONNRESET == nErrorCode;
#endif
}

static bool IsIgnoreSocketError(int &nErrorCode) SR_UNUSED;
static bool IsIgnoreSocketError(int &nErrorCode)
{
#ifdef WIN32
  nErrorCode = GetLastError();
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

static bool IsIgnoreSocketError_UDP(int &nErrorCode) SR_UNUSED;
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
    if(ECONNRESET == nErrorCode)
    {
        return true;
    }
#endif
    return false;
}
typedef enum __SOCKET_TYPE { T_UDP = 0, T_TCP, T_PROTOCOL_DEFAULT, T_PROTOCOL_NULL } SOCKET_TYPE;

inline std::string ConvertIp(uint32_t ip_little_end) SR_UNUSED;
inline std::string ConvertIp(uint32_t ip_little_end)
{
  struct in_addr in;
  in.s_addr = htonl(ip_little_end);
  char ipbuf[32] = {0};
#ifdef _WIN_XP_
  win32_inet_ntop(AF_INET, &in, ipbuf, 32);//20190708
#else
  inet_ntop(AF_INET, &in, ipbuf, 32);
#endif
  
  return std::string(ipbuf);
}

#define IPV42STR(INADDR, IPSTR) \
char IPSTR[32] = { 0 };\
inet_ntop(AF_INET, &INADDR, IPSTR, 32);

#endif //_NET_COMMON_H_


