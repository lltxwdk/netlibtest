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
volatile,  易失的；易变的；易挥发的。那么用这个关键词修饰的C/C++变量，应该也能够体现出”易变”的特征
易变性。所谓的易变性，在汇编层面反映出来，就是两条语句，下一条语句不会直接使用上一条语句对应的volatile变量的寄存器内容，而是重新从内存中读取。

Volatile：不可优化的
“不可优化”特性。volatile告诉编译器，不要对我这个变量进行各种激进的优化，甚至将变量直接消除，保证程序员写在代码中的指令，一定会被执行。

Volatile：顺序性
C/C++ Volatile变量，与非Volatile变量之间的操作，是可能被编译器交换顺序的
C/C++ Volatile变量间的操作，是不会被编译器交换顺序的。目前，市场上有各种不同体系架构的CPU产品，CPU本身为了提高代码运行的效率，也会对代码的执行顺序进行调整，这就是所谓的CPU Memory Model (CPU内存模型

C/C++ Volatile关键词的第三个特性：”顺序性”，能够保证Volatile变量间的顺序性，编译器不会进行乱序优化。
Volatile变量与非Volatile变量的顺序，编译器不保证顺序，可能会进行乱序优化。同时，C/C++ Volatile关键词，并不能用于构建happens-before语义，因此在进行多线程程序设计时，要小心使用volatile，不要掉入volatile变量的使用陷阱之中。


*/