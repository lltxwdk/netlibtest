#ifndef __EVENT_HANDLER_H__
#define __EVENT_HANDLER_H__
class EventHandler
{
public:
	virtual int GetEventFd() = 0;
	virtual bool IsNeedWriteEvent() = 0;
	virtual void HandleEvent(bool ev_err, bool ev_read, bool ev_write) = 0;
};

#endif