#include "TcpListenner.h"

TcpListenner::TcpListenner(EventLoop* ent_loop_ins, ITcpListenEvent* ent_handler, TcpConnCreator& tcp_creator)
{
	tcp_creator_ = tcp_creator;
	listen_sock_ = this;
	ent_loop_ins_ = ent_loop_ins;
	ent_handler_ = ent_handler;
}

TcpListenner::~TcpListenner()
{

}

bool TcpListenner::Listen(const char* listen_ip, unsigned short listen_port, bool ipv6_mode)
{
	listen_sock_.SetIpv6(ipv6_mode);
	if (!listen_sock_.Listen(listen_ip,listen_port))
	{
		return false;
	}
	if (!ent_loop_ins_->AddEventFd(FD_TCP_LISTEN,&listen_sock_))
	{
		return false;
	}
	return true;
}
void TcpListenner::Close()
{

	ent_loop_ins_->DelEventFd(&listen_sock_);
	listen_sock_.Close();
}

void TcpListenner::Release()
{
	delete this;
}

void TcpListenner::OnAccept(int sock_fd, char* remote_ip, uint16_t remote_port)
{
	tcp_creator_.CreatNewConn(sock_fd, remote_ip, remote_port, ent_handler_);
}

void TcpListenner::OnNetError(int err_code)
{
	ent_handler_->OnNetError(err_code);
}