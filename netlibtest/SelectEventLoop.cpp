#include "SelectEventLoop.h"
#include "EventHandler.h"

SelectEventLoop::SelectEventLoop()
{
	notify_socket_fd_ = INVALID_SOCKET;
	running_ = false;
}

SelectEventLoop::~SelectEventLoop()
{
}

bool SelectEventLoop::Start()
{
	if (!UpdateNotifySocket())
	{
		return false;
	}

	unsigned int thd = 0;
	thread_.BeginThread(ThreadFun, this, thd, "SelectEventLoop");
	return true;
}

void SelectEventLoop::Stop()
{
	if (!running_)
	{
		return;
	}
	running_ = false;

	Wakeup();
	while (thread_.IsRunning())
	{
		CBaseThread::Sleep(10);
	}
}

bool SelectEventLoop::AddEventFd(FD_TYPE fd_tpe, EventHandler* handler)
{
	assert(handler);
	ModifyHandlerSet(handler, 1);
	Wakeup();
	return true;
}

void SelectEventLoop::DelEventFd(EventHandler* handler)
{
	assert(handler);
	if (NULL == handler)
	{
		return;
	}
	ModifyHandlerSet(handler, 0);
	//防止handler 在set  Modify 0 之后，释放， 导致的没有及时更新handler，获取崩溃
	//by cl.zhan
	SyncHandlerSet();
	Wakeup();
}

void SelectEventLoop::SetWriteEventForFd(EventHandler* handler, bool need)
{

}

bool SelectEventLoop::UpdateNotifySocket()
{
	if (!notify_socket_.CreateAsyncSocket(false))
	{
		return false;
	}
	uint16_t socket_port = 0;
	char local_ip[20] = "127.0.0.1";
	if (!notify_socket_.Bind(local_ip, socket_port))
	{
		notify_socket_.CloseSocket();
		return false;
	}
	if (notify_socket_fd_ != notify_socket_.GetSocketFd())
	{
		close_socket(notify_socket_fd_)
	}
	notify_socket_fd_ = notify_socket_.GetSocketFd();

	notify_addr_in_.sin_family = AF_INET;
	notify_addr_in_.sin_addr.s_addr = ConvertIPV4ToInt("127.0.0.1");
	notify_addr_in_.sin_port = htons(socket_port);
	return true;
}

uint32_t SelectEventLoop::ThreadFun(void* arg)
{
	SelectEventLoop* net_event = (SelectEventLoop*)arg;
	net_event->MyRun();

	return 0;
}

void SelectEventLoop::MyRun()
{
	running_ = true;
	while (running_)
	{
		EventHandle();
	}

	running_ = false;
}

void SelectEventLoop::Wakeup()
{
	char data = 'n';
	::sendto(notify_socket_fd_, &data, 1, 0, (struct sockaddr*)&notify_addr_in_, sizeof(notify_addr_in_));
}

void SelectEventLoop::EventHandle()
{
	int socket_max_fd = INVALID_SOCKET;

	fd_set fds_read;
	fd_set fds_write;
	fd_set fds_error;
	FD_ZERO(&fds_read);
	FD_ZERO(&fds_write);
	FD_ZERO(&fds_error);
	FillFdset(&fds_read, &fds_write, &fds_error, socket_max_fd);

	struct timeval tempval;
	tempval.tv_sec = 0;
	tempval.tv_usec = 1000 * 100;
	int result = select(socket_max_fd + 1, &fds_read, &fds_write, &fds_error, &tempval);
	if (0 > result)
	{
		int err = GetErrCode();
		//printf("select error:%d\n", err);
		return;
	}
	if (0 == result)
	{
		//timeout
		SyncHandlerSet();
		return;
	}
	if (FD_ISSET(notify_socket_fd_, &fds_read))
	{
		char notify_data;
		struct sockaddr_in from;
		socklen_t len = sizeof(from);
		int err_code = 0;
		int ret = ::recvfrom(notify_socket_fd_, &notify_data, 1, 0, (struct sockaddr*)&from, &len);
		if (ret < 0 && !IsIgnoreSocketError(err_code))
		{
			UpdateNotifySocket();
			LogINFO("notify fd error:%d", notify_socket_fd_);
		}
		SyncHandlerSet();
	}

	int sock_fd = INVALID_SOCKET;
	EventHandler* handler = NULL;
	for (size_t i = 0; i < handler_arr_.size(); i++)
	{
		handler = handler_arr_[i];
		assert(handler);
		sock_fd = handler->GetEventFd();
		if (INVALID_SOCKET == sock_fd)
		{
			LogINFO("socket is invalided, notify fd:%d", notify_socket_fd_);
			continue;
		}

		bool ev_err = FD_ISSET(sock_fd, &fds_error);
		bool ev_read = FD_ISSET(sock_fd, &fds_read);
		bool ev_write = FD_ISSET(sock_fd, &fds_write);
		//if (FD_ISSET(sock_fd, &fds_read))
		//{
		//    handler->HandleReadEvent();
		//}

		//if (FD_ISSET(sock_fd, &fds_write))
		//{
		//    handler->HandleWriteEvent();
		//}

		//if (FD_ISSET(sock_fd, &fds_error))
		//{
		//    handler->HandleErrorEvent();
		//}
		handler->HandleEvent(ev_err, ev_read, ev_write);
	}
}

void SelectEventLoop::SyncHandlerSet()
{
	CCriticalAutoLock guard(pending_handlers_lock_);
	if (pending_handlers.empty())
	{
		return;
	}
	while (!pending_handlers.empty())
	{
		HandlerModify& modify = *pending_handlers.begin();
		if (0 == modify.op_)
		{
			handler_set_.erase(modify.handler_);
		}
		else
		{
			assert(1 == modify.op_);
			handler_set_.insert(modify.handler_);
		}
		pending_handlers.pop_front();
	}

	//update handler set
	handler_arr_.clear();
	std::set<EventHandler*>::iterator it = handler_set_.begin();
	for (; it != handler_set_.end(); it++)
	{
		handler_arr_.push_back(*it);
	}
}

void SelectEventLoop::FillFdset(fd_set* fds_read, fd_set* fds_write, fd_set* fds_error, int& socket_max_fd)
{
	socket_max_fd = notify_socket_fd_;
	FD_SET(notify_socket_fd_, fds_read);

	int sock_fd = INVALID_SOCKET;
	EventHandler* handler = NULL;
	for (size_t i = 0; i < handler_arr_.size(); i++)
	{
		handler = handler_arr_[i];
		assert(handler);
		sock_fd = handler->GetEventFd();
		if (INVALID_SOCKET == sock_fd)
		{
			continue;
		}
		if (sock_fd > socket_max_fd)
		{
			socket_max_fd = sock_fd;
		}
		FD_SET(sock_fd, fds_read);
		FD_SET(sock_fd, fds_error);
		if (handler->IsNeedWriteEvent())
		{
			FD_SET(sock_fd, fds_write);
		}
	}
}