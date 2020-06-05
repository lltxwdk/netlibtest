#ifndef __TCP_NETINTERFACE_H__
#define __TCP_NETINTERFACE_H__

//tcp 事件回调接口 
class ITcpEvent
{
public:
	//tcp链接成功事件通知
	virtual void OnConnected() = 0;

	//链接断开事件通知
	virtual void OnDisconnect(int err_code) = 0;

	//数据到达事件通知, 接口实现者需要注意,返回值表明消费掉的数据长度, 最小为0, 最大为data_len
	virtual int OnRecvData(char* data, int data_len) = 0;
};

//tcp传输接口
class ITcpConn
{
public:
	//初始化, 指明发送和接收buf大小, 
	virtual bool Init(ITcpEvent* ent_handler, int max_sendbuf_len, int max_recvbuf_len) = 0;

	//建立连接, 失败返回false 
	//默认ipv6_mode = false 表示当前为 ipv4 模式，若 ipv6_mode = true 为 ipv6 模式 
	virtual bool Connect(char* remote_ip, unsigned short remote_port, bool ipv6_mode = false) = 0;

	//发送数据, 返回值: 大于0表示成功, 0 表示sendbuf已满, 小于0表示出现错误需要重新建立连接
	virtual int SendData(char* data, int data_len) = 0;

	//关闭连接
	virtual void Close() = 0;

	//释放对象内存, 调用此接口后, 对象指针不能在使用
	virtual void Release() = 0;
};


//tcp 侦听事件回调
class ITcpListenEvent
{
public:
	//接受新的链接, 实现者需首先对new_tcpconn 执行Init方法, 初始化tcp链接
	virtual void OnAccept(ITcpConn* new_tcpconn, char* remote_ip, unsigned short remote_port) = 0;

	//侦听socket发生错误
	virtual void OnNetError(int err_code) = 0;
};

//tcp 侦听接口
class ITcpListenner
{
public:
	//启动侦听, 失败返回false
	//默认ipv6_mode = false 表示当前为 ipv4 模式，若 ipv6_mode = true 为 ipv6 模式 
	virtual bool Listen(const char* listen_ip, unsigned short listen_port, bool ipv6_mode = false) = 0;

	//关闭侦听
	virtual void Close() = 0;

	//释放对象内存, 调用此接口后, 对象指针不能在使用
	virtual void Release() = 0;
};

//tcp trans module 接口
class ITcpTransLib
{
public:
	//初始化, 参数指明线程数和事件分发模型(select或epoll)
	virtual bool Init(int thread_num, int mode) = 0;

	//反初始化, 停止所有线程
	virtual void Uninit() = 0;

	//创建tcp侦听socket
	virtual ITcpListenner* CreateTcpListenner(ITcpListenEvent* ent_handler) = 0;

	//创建主动链接
	virtual ITcpConn* CreateTcpConn() = 0;
};

ITcpTransLib* CreateTcpTransLib();
void DeleteTcpTransLib(ITcpTransLib* trans_lib);
#endif