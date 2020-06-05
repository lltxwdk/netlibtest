#pragma once
#include "TypeDef.h"
#include "BaseThread.h"
#include "Socket.h"
#include "EventLoop.h"

class SelectEventLoop : public EventLoop
{
	friend class SelectEventLoop_UTest;
	typedef struct _tagHandlerModify
	{
		EventHandler* handler_;
		int op_;//1 add, 0 del;
		_tagHandlerModify()
		{
			handler_ = NULL;
			op_ = 0;
		}
	} HandlerModify;
public:
	SelectEventLoop();
	virtual ~SelectEventLoop();

public:
	virtual bool Start();
	virtual void Stop();
	virtual bool AddEventFd(FD_TYPE fd_tpe, EventHandler* handler);
	virtual void DelEventFd(EventHandler* handler);
	virtual void SetWriteEventForFd(EventHandler* handler, bool need);
private:
	static uint32_t ThreadFun(void* arg);
	void MyRun();
	void EventHandle();
	void FillFdset(fd_set* fds_read, fd_set* fds_write, fd_set* fds_error, int& socket_max_fd);
	bool UpdateNotifySocket();
	void Wakeup();
	void SyncHandlerSet();
	void ModifyHandlerSet(EventHandler* handler, int op)
	{
		HandlerModify modify;
		modify.handler_ = handler;
		modify.op_ = op;
		CCriticalAutoLock guard(pending_handlers_lock_);
		pending_handlers.push_back(modify);
	}
private:
	AsyncSocket notify_socket_;
	int notify_socket_fd_;
	struct sockaddr_in notify_addr_in_;

	CCriticalSection pending_handlers_lock_;
	std::list<HandlerModify> pending_handlers;


	std::set<EventHandler*> handler_set_;
	std::vector<EventHandler*> handler_arr_;

	CBaseThread thread_;
	volatile bool running_;
};