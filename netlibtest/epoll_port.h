#ifndef __EPOLL_PORT_H__
#define __EPOLL_PORT_H__
#ifdef LINUX
#include <sys/epoll.h>
#include <sys/poll.h>
#else
#include "TypeDef.h"
//operate
#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_MOD 2
#define EPOLL_CTL_DEL 3

//events
#define EPOLLIN 1
//The associated file is available for read(2) operations.

#define EPOLLOUT 2
//The associated file is available for write(2) operations.

#define EPOLLRDHUP 3/*(since Linux 2.6.17)
Stream  socket  peer  closed connection, or shut down writing half of connection.  (This flag is espe -
cially useful for writing simple code to detect peer shutdown when using Edge Triggered monitoring.)*/

#define EPOLLPRI 4
//There is urgent data available for read(2) operations.

#define EPOLLERR 5
//Error condition happened on the associated file descriptor.epoll_wait(2) will always wait  for  this
//event; it is not necessary to set it in events.

#define EPOLLHUP 6
//Hang up happened on the associated file descriptor.epoll_wait(2) will always wait for this event; it
//is not necessary to set it in events.

#define EPOLLET 7
//Sets the Edge Triggered behavior for the associated file descriptor.The default behavior  for  epoll
//is  Level  Triggered.See epoll(7) for more detailed information about Edge and Level Triggered event
//distribution architectures.

#define EPOLLONESHOT 8/*(since Linux 2.6.2)
Sets the one - shot behavior for the associated file descriptor.This means  that  after  an  event  is
pulled  out  with  epoll_wait(2)  the  associated  file descriptor is internally disabled and no other
events will be reported by the epoll interface.The user must call epoll_ctl() with EPOLL_CTL_MOD  to
re - arm the file descriptor with a new event mask.*/

//flags
#define EPOLL_CLOEXEC 0

typedef union epoll_data {
	void        *ptr;
	int          fd;
	uint32_t   u32;
	uint64_t   u64;
} epoll_data_t;

struct epoll_event {
	uint32_t   events;      /* Epoll events */
	epoll_data_t data;        /* User data variable */
};
static int epoll_create(int size){ return -1; }
static int epoll_create1(int flags){ return -1; }
static int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event){ return -1; }
static int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout){ return -1; }
#endif
#endif

