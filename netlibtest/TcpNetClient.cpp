#include "TraceLog.h"
#include "TcpNetClient.h"
#include "PackDef.h"
TcpNetClient::TcpNetClient(ITcpNetEventLoop* net_event_loop, PduHandler* pdu_handler)
    :NetClientBase(pdu_handler),
    tcp_event_loop_(net_event_loop)
{
    m_bConnected = false;
}
TcpNetClient::TcpNetClient(ITcpNetEventLoop* net_event_loop, PduHandler* pdu_handler,
    uint32_t max_sendbuf_len, uint32_t max_recvbuf_len)
    : NetClientBase(pdu_handler),
    tcp_event_loop_(net_event_loop),
    pdu_parser_(max_sendbuf_len, max_recvbuf_len)
{
    m_bConnected = false;
}
TcpNetClient::~TcpNetClient(void)
{
}

bool TcpNetClient::Open(uint32_t remote_ip, uint16_t remote_port, uint32_t local_ip, uint16_t local_port)
{
    if (NULL != socket_fd_)
    {
        LogERR("Create tcp socket error.socket_fd_ is no nul:%d", socket_fd_);
        return false;
    }

    pdu_parser_.Reset();
    m_bConnected = false;
    local_ip_ = local_ip;
    local_port_ = local_port;
    socket_fd_ = tcp_event_loop_->CreateSocket();
    if( 0 >= socket_fd_ )
    {
        LogERR("Create tcp socket error.");
        return false;
    }
    if (0 > tcp_event_loop_->Bind(socket_fd_, local_ip_, local_port_))
    {
        tcp_event_loop_->CloseSocket(socket_fd_);
        socket_fd_ = NULL;
        LogERR("Bind tcp socket error.");
        return false;
    }
    INetTcpEvent* pTcp = this;
    //tcp_event_loop_->SetSocketContext(socket_fd_, pTcp);
    if (tcp_event_loop_->Connect(socket_fd_, pTcp, remote_ip, remote_port) < 0)
    {
        Close();
        LogERR("Tcp connect socket error.");
        return false;
    }

    remote_ip_ = remote_ip;
    remote_port_ = remote_port;
	socket_closed_time_ = Utils::Time(); //reset in OnConnected()
    return true;
}

bool TcpNetClient::Open(LPSOCKET socket, uint32_t remote_ip, uint16_t remote_port)
{
    if (socket == NULL)
      return false;

    socket_fd_ = socket;
    INetTcpEvent* pTcp = this;
    tcp_event_loop_->SetSocketContext(socket_fd_, pTcp);
    m_bConnected = true;
    remote_ip_ = remote_ip;
    remote_port_ = remote_port;
	socket_closed_time_ = 0;
    return true;
}

void TcpNetClient::PeerClose()
{
    if (NULL == socket_fd_)
    {
        return;
    }
    InetAddress addr(remote_ip_, remote_port_);
    InetAddress local(local_ip_, local_port_);
    LogINFO("PeerClose:Close local socket:%s, remote addr:%s", local.AsString().c_str(), addr.AsString().c_str());
    tcp_event_loop_->CloseSocket(socket_fd_);
    ResetSocket(false);
}

void TcpNetClient::Close()
{
    if( NULL == socket_fd_ )
    {
        return;
    }
    InetAddress addr(remote_ip_, remote_port_);
    InetAddress local(local_ip_, local_port_);
    LogINFO("Close:Close local socket:%s, remote addr:%s", local.AsString().c_str(), addr.AsString().c_str());
    tcp_event_loop_->CloseSocket(socket_fd_);
    ResetSocket(true);
}

void TcpNetClient::ResetSocket(bool bActive)
{
    if (!bActive)
    {
        pdu_handler_->OnClosed((void*)socket_fd_);
    }

    socket_fd_ = NULL;
    m_bConnected = false;
    local_ip_ = 0;
    local_port_ = 0;
    socket_closed_time_ = Utils::Time();
}

bool TcpNetClient::IsClosed()
{
    return (NULL == socket_fd_);
}

void TcpNetClient::GetLocalAddr( uint32_t &local_ip, uint16_t &local_port )
{
    if (NULL == socket_fd_)
    {
        local_ip = 0;
        local_port = 0;
        return;
    }

    local_ip = local_ip_;
    local_port = local_port_;
}

int TcpNetClient::SendRawData(char* data, uint32_t data_len)
{
    int ret = 0;
    if(pdu_parser_.SendTcpPdu(data, data_len, PACK_CMD_ID_RTP_RTCP_PACKET, this))
    {
        ret = data_len;
    }
    return ret;
}

int TcpNetClient::SendPduData(uint16_t cmd_id, char* data, uint32_t data_len)
{
    int ret = 0;
    if(pdu_parser_.SendTcpPdu(data, data_len, cmd_id, this))
    {
        ret =  data_len;
    }
    return ret;
}

int TcpNetClient::SendPduData(uint16_t cmd_id, char* data, uint32_t data_len, uint8_t id)
{
    int ret = 0;
    if(pdu_parser_.SendTcpPdu(data, data_len, cmd_id, this))
    {
        ret = data_len;
    }
    return ret;
}

bool TcpNetClient::SendPduData(PDUSerialBase& pdu_serial)
{
    int ret = pdu_parser_.SendTcpPdu(pdu_serial, this);

    return (ret > 0);
}

//implement INetTcpEvent
//int TcpNetClient::OnReciveData(LPSOCKET socket_fd, char* data, uint32_t data_len, void* context)
void TcpNetClient::HandleSocketEvent(LPSOCKET socket, EM_SOCKET_EVENT ent, void* context)
{
    if (socket != socket_fd_)
    {
        LogWARNING("TcpNetClient HandleSocketEvent may be closed already, socket:%p, socket_fd_:%p", socket, socket_fd_);
        return;
    }

    int ret = -1;
    char msg[PACK_MAX_LEN];
    if (ent == EVENT_R)
    {
        int data_len = tcp_event_loop_->RecvData(socket, msg, PACK_MAX_LEN);
        if (0 == data_len)
        {
            InetAddress addr(remote_ip_, remote_port_);
            LogINFO("Recv tcp code is 0, addr:%s", addr.AsString().c_str());
            PeerClose();
            return;
        }

        if (data_len < 0)
        {
            int nError = 0;
            if (IsIgnoreSocketError(nError))
            {
                return;
            }
            else
            {
                InetAddress addr(remote_ip_, remote_port_);
                LogINFO("Recv tcp data_len:%d, nError=%d, addr:%s", data_len, nError, addr.AsString().c_str());
                PeerClose();
                return;
            }
        }
        if (!pdu_parser_.ParseTcpPdu(msg, data_len, this))
        {
            InetAddress addr(remote_ip_, remote_port_);
            LogERR("ParseTcpPdu Recv tcp data_len:%d, addr:%s", data_len, addr.AsString().c_str());
            PeerClose();
            return;
        }
    }
    else if (ent == EVENT_W)
    {
        if (socket != socket_fd_)
        {
            LogERR("TcpNetClient EVENT_W socket:%p, socket_fd_:%p", socket, socket_fd_);
            return;
        }

        ret = pdu_parser_.Flush(this);
        if (0 == ret)
        {
            //send buf is empty
            pdu_handler_->OnSendPdu();
        }
        else if (-2 == ret)
        {
            int nError = 0;
            if (IsIgnoreSocketError(nError))
            {
                return;
            }
            else
            {
                InetAddress addr(remote_ip_, remote_port_);
                LogINFO("Recv tcp EVENT_W ret -2, nError=%d, addr:%s", nError, addr.AsString().c_str());
                PeerClose();
                return;
            }
        }
    }
    else if (ent == EVENT_E)
    {
        int nError = 0;
        if (IsIgnoreSocketError(nError))
        {
            return;
        }
        
        InetAddress addr(remote_ip_, remote_port_);
        LogINFO("Recv tcp error code:%d, addr:%s", nError, addr.AsString().c_str());
        PeerClose();
        return;
    }
}

/*void TcpNetClient::OnSendReady(LPSOCKET socket_fd, void* context)
{
    if (socket_fd != socket_fd_)
    {
        return;
    }

    if(0 == pdu_parser_.Flush(this))
    {
        //send buf is empty
        pdu_handler_->OnSendPdu();
    }
}*/

bool TcpNetClient::IsNeedSend(LPSOCKET socket_fd, void* context)
{
    if (socket_fd != socket_fd_)
    {
        LogERR("TcpNetClient IsNeedSend socket_fd:%p, socket_fd_:%p", socket_fd, socket_fd_);
        //assert(false);
        return false;
    }

    return pdu_parser_.isNeedSend();
}

void TcpNetClient::OnClosed(LPSOCKET socket_fd, void* context)
{
    InetAddress addr(remote_ip_, remote_port_);
    LogINFO("Recv tcp on closed event, addr:%s", addr.AsString().c_str());
    if (socket_fd != socket_fd_)
    {
        LogWARNING("TcpNetClient socket_fd:%p, socket_fd_:%p, may be close and open again", socket_fd, socket_fd_);
        return;
    }

    ResetSocket(false);
}

void TcpNetClient::OnHandleTcpPdu(uint16_t cmd_id, char* pdu_data, uint32_t data_len)
{
    pdu_handler_->OnHandlePdu(cmd_id, pdu_data, data_len, remote_ip_, remote_port_);
}

void TcpNetClient::OnConnected(LPSOCKET socket_fd, void* context)
{
    LogDEBUG("TcpNetClient::OnConnected, socket_fd:%p", socket_fd);
    tcp_event_loop_->SetSocketContext(socket_fd, context);
    m_bConnected = true;

	socket_closed_time_ = 0;

    int ret = pdu_parser_.Flush(this);
    if (0 == ret)
    {
        //send buf is empty
        pdu_handler_->OnSendPdu();
    }
    else if (-2 == ret)
    {
        int nError = 0;
        if (IsIgnoreSocketError(nError))
        {
            return;
        }
        else
        {
            InetAddress addr(remote_ip_, remote_port_);
            LogERR("Recv tcp OnConnected error:%d, addr:%s", nError, addr.AsString().c_str());
            PeerClose();
            return;
        }
    }
}

int TcpNetClient::Send(char* data, uint32_t data_len)
{
    if( !m_bConnected )
    {
        //LogERR("Tcp is not ready.Please wait...");
        return -1;
    }

    if (socket_fd_ == NULL)
    {
        InetAddress addr(remote_ip_, remote_port_);
        LogERR("TcpNetClient send error,socket fd is null, remote:%s ", addr.AsString().c_str());
        return -1;
    }

    return tcp_event_loop_->SendData(socket_fd_, data, data_len);
}


