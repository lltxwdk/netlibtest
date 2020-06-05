#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

typedef enum
{
	FD_TCP_LISTEN,
	FD_TCP_NORMAL,
	FD_UDP
} FD_TYPE;
class EventHandler;
class EventLoop
{
public:
	EventLoop(){}
	virtual ~EventLoop(){}
	virtual bool Start() = 0;
	virtual void Stop() = 0;
	virtual bool AddEventFd(FD_TYPE fd_tpe, EventHandler* handler) = 0;
	virtual void DelEventFd(EventHandler* handler) = 0;
	virtual void SetWriteEventForFd(EventHandler* handler, bool need) = 0;
};

// model's value must be ASYNC_MODEL_SELECT or ASYNC_MODEL_EPOLL.
//return NULL if error is occurred
EventLoop* CreateEventLoop(int model);

//delete EventLoop instance
void DeleteEventLoop(EventLoop*& ins);

#endif
