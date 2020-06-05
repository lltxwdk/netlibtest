#include "NetLibCommon.h"
#include "EpollEventLoop.h"
#include "SelectEventLoop.h"

EventLoop* CreateEventLoop(int model)
{
#ifdef WIN32
	model = ASYNC_MODEL_SELECT;
#endif
	if (model == ASYNC_MODEL_SELECT)
	{
		return new SelectEventLoop();
	} 
	assert(model == ASYNC_MODEL_EPOLL);

	return new EpollEventLoop();
}

void DeleteEventLoop(EventLoop*& ins)
{
	delete ins;
	ins = NULL;
}