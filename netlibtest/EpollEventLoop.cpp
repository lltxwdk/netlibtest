#include "NetPortable.h"
#include "TraceLog.h"
#include "EpollEventLoop.h"
#include "EventHandler.h"

#include "epoll_port.h"

EpollEventLoop::EpollEventLoop()
{
	epoll_fd_ = -1;

}

EpollEventLoop::~EpollEventLoop()
{
}

bool EpollEventLoop::Start()
{
	assert(-1 == epoll_fd_);
	epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
	if (epoll_fd_ < 0)
	{
		return false;
	}
	unsigned int thredid = 0;
	thread_.BeginThread(ThreadFun, this, thredid, "EpollEventLoop");
	return true;
}

void EpollEventLoop::Stop()
{
	if (!running_)
	{
		return;
	}
	running_ = false;

	while (thread_.IsRunning())
	{
		CBaseThread::Sleep(10);
	}
	close_socket(epoll_fd_);
	epoll_fd_ = -1;
}

bool EpollEventLoop::AddEventFd(FD_TYPE fd_tpe, EventHandler* handler)
{
	assert(running_);
	assert(handler);

	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;

	ev.data.ptr = (void *)handler;
	int fd = handler->GetEventFd();
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD,fd,&ev) < 0)
	{
		LogERR("AddEventFd fail! error code:%u, discrbie:%s", errno, strerror(errno));
		return false;
	}
	return true;
}

void EpollEventLoop::DelEventFd(EventHandler* handler)
{
	int fd = handler->GetEventFd();
	epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL);
}

void EpollEventLoop::SetWriteEventForFd(EventHandler* handler, bool need)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	if (need)
	{
		ev.events |= EPOLLOUT;
	}
	ev.data.ptr = (void *)handler;
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, handler->GetEventFd(), &ev) < 0)
	{
		LogERR("mod state is %u. errno = %d, reason is %s\n", ev.events, errno, strerror(errno));
	}

}

uint32_t EpollEventLoop::ThreadFun(void* arg)
{
	EpollEventLoop* net_event = (EpollEventLoop*)arg;
	net_event->MyRun();
	return 0;
}

void EpollEventLoop::MyRun()
{
	struct epoll_event incoming_events[100];
	int event_cnt = 0;
	int cur_event = 0;
	EventHandler* handler = NULL;
	running_ = true;

	while (running_)
	{
		event_cnt = epoll_wait(epoll_fd_, incoming_events, 100, 100);
		if (event_cnt < 0)
		{
			if (EINTR == errno)
			{
				continue;
			}

			LogERR("epoll_wait():%s", strerror(errno));
			break;
		}
		if (event_cnt == 0)
		{
			continue;
		}
		assert(event_cnt > 0);
		for (int i = 0; i < event_cnt; i++)
		{
			handler = (EventHandler*)(incoming_events[i].data.ptr);
			assert(handler);
			cur_event = incoming_events[i].events;
			handler->HandleEvent(cur_event & EPOLLERR, cur_event & EPOLLIN, cur_event & EPOLLOUT)
		}

	}
	return false;
}