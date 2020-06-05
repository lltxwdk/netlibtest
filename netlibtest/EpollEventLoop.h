#pragma once

#include "TypeDef.h"
#include "BaseThread.h"
#include "EventLoop.h"

class EpollEventLoop : public EventLoop
{
public:
	EpollEventLoop();
	~EpollEventLoop();
private:
	virtual bool Start();
	virtual void Stop();
	virtual bool AddEventFd(FD_TYPE fd_tpe, EventHandler* handler);
	virtual void DelEventFd(EventHandler* handler);
	virtual void SetWriteEventForFd(EventHandler* handler, bool need);

private:
	static uint32_t ThreadFun(void* arg);
	void MyRun();
private:
	int epoll_fd_;

	CBaseThread thread_;
	volatile bool running_;

};

/*
volatile,  ��ʧ�ģ��ױ�ģ��׻ӷ��ġ���ô������ؼ������ε�C/C++������Ӧ��Ҳ�ܹ����ֳ����ױ䡱������
�ױ��ԡ���ν���ױ��ԣ��ڻ����淴ӳ����������������䣬��һ����䲻��ֱ��ʹ����һ������Ӧ��volatile�����ļĴ������ݣ��������´��ڴ��ж�ȡ��

Volatile�������Ż���
�������Ż������ԡ�volatile���߱���������Ҫ��������������и��ּ������Ż�������������ֱ����������֤����Աд�ڴ����е�ָ�һ���ᱻִ�С�

Volatile��˳����
C/C++ Volatile���������Volatile����֮��Ĳ������ǿ��ܱ�����������˳���
C/C++ Volatile������Ĳ������ǲ��ᱻ����������˳��ġ�Ŀǰ���г����и��ֲ�ͬ��ϵ�ܹ���CPU��Ʒ��CPU����Ϊ����ߴ������е�Ч�ʣ�Ҳ��Դ����ִ��˳����е������������ν��CPU Memory Model (CPU�ڴ�ģ��

C/C++ Volatile�ؼ��ʵĵ��������ԣ���˳���ԡ����ܹ���֤Volatile�������˳���ԣ�������������������Ż���
Volatile�������Volatile������˳�򣬱���������֤˳�򣬿��ܻ���������Ż���ͬʱ��C/C++ Volatile�ؼ��ʣ����������ڹ���happens-before���壬����ڽ��ж��̳߳������ʱ��ҪС��ʹ��volatile����Ҫ����volatile������ʹ������֮�С�


*/