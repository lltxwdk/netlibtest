#ifndef __NETLIB_COMMON_H__
#define __NETLIB_COMMON_H__

///common macro define
//async model define
#define ASYNC_MODEL_SELECT  1
#define ASYNC_MODEL_EPOLL   2

int IPV4StrToInt(const char* ip);
void IPV4IntToStr(int ip_be, char ip[32]);
#endif