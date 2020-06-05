#include "UdpNetClient.h"
#include "TraceLog.h"

UdpNetClient::UdpNetClient( IUdpNetEventLoop* net_event_loop, PduHandler* pdu_handler )
:NetClientBase(pdu_handler),
net_event_loop_(net_event_loop),
running_(false)
{
}

UdpNetClient::~UdpNetClient(void)
{
    
}

bool UdpNetClient::Open(uint32_t remote_ip, uint16_t remote_port, uint32_t local_ip, uint16_t local_port)
{
    if (!IsClosed())
    {
        LogERR("Open socket error, socket not closed");
        return false;
    }
    socket_fd_ = net_event_loop_->CreateSocket();
    if( NULL == socket_fd_ )
    {
        LogERR("Open socket error, invalid socket");
        return false;
    }

    local_ip_ = local_ip;
    local_port_ = local_port;
    if( 0 > net_event_loop_->Bind(socket_fd_, local_ip_, local_port_) )
    {
        net_event_loop_->CloseSocket(socket_fd_);
        socket_fd_ = NULL;
        LogERR("udp open error, bind error.");
        return false;
    }
    INetUdpEvent* pUdp = this;
    net_event_loop_->SetSocketContext(socket_fd_, pUdp);

    remote_ip_ = remote_ip;
    remote_port_ = remote_port;

	socket_closed_time_ = 0;
    running_ = true;

    return true;
}

void UdpNetClient::Close()
{
    if( NULL != socket_fd_ )
    {
        net_event_loop_->CloseSocket(socket_fd_);
        
    }

    ResetSocket();
}

void UdpNetClient::ResetSocket()
{
    socket_fd_ = NULL;
    socket_closed_time_ = Utils::Time();
}

bool UdpNetClient::IsClosed()
{
    return ((NULL == socket_fd_) && (!running_));
}

int UdpNetClient::SendRawData(char* data, uint32_t data_len)
{
    return SendData(data, data_len, remote_ip_, remote_port_);
}

int UdpNetClient::SendRawData(char* data, uint32_t data_len, uint32_t remote_ip, uint16_t remote_port)
{
    return SendData(data, data_len, remote_ip, remote_port);
}

int UdpNetClient::SendPduData(uint16_t cmd_id, char* data, uint32_t data_len)
{
    ParsePdu parse;

    char nBuffer[PACK_MAX_LEN] = {0};
   // uint16_t nLength = PACK_MAX_LEN;
    int len = parse.packPduBuf(data, data_len, cmd_id, nBuffer);
    return SendRawData(nBuffer, len);
}

int UdpNetClient::SendPduData(uint16_t cmd_id, char* data, uint32_t data_len, uint8_t id)
{
  //ParsePdu2 parse;

    char nBuffer[PACK_MAX_LEN] = {0};
    //uint16_t nLength = PACK_MAX_LEN;
    ParsePdu parse;
    int len = parse.packPduBuf(data, data_len, cmd_id, id, nBuffer);
    return SendRawData(nBuffer, len);
}

int UdpNetClient::SendPduData(uint16_t cmd_id, char* data, uint32_t data_len, uint32_t remote_ip, uint16_t remote_port)
{
    ParsePdu parse;

    char nBuffer[PACK_MAX_LEN] = {0};
    //uint16_t nLength = PACK_MAX_LEN;
    int len = parse.packPduBuf(data, data_len, cmd_id, nBuffer);
    return SendRawData(nBuffer, len, remote_ip, remote_port);
}

int UdpNetClient::SendPduData(uint16_t cmd_id, char* data, uint32_t data_len, uint8_t id, uint32_t remote_ip, uint16_t remote_port)
{
    char nBuffer[PACK_MAX_LEN] = {0};
    //uint16_t nLength = PACK_MAX_LEN;
    ParsePdu parse;
    int len = parse.packPduBuf(data, data_len, cmd_id, id, nBuffer);
    return SendRawData(nBuffer, len, remote_ip, remote_port);
}

bool UdpNetClient::SendPduData(PDUSerialBase& pdu_serial)
{
    ParsePdu parse;

    char nBuffer[PACK_MAX_LEN] = {0};
    uint16_t nLength = PACK_MAX_LEN;
    if(!parse.packPduBuf(pdu_serial, nBuffer, nLength))
    {
        return false;
    }

    return (SendData(nBuffer, nLength, remote_ip_, remote_port_) > 0);
}

int UdpNetClient::SendPduData(PDUSerialBase& pdu_serial, uint32_t remote_ip, uint16_t remote_port)
{
    ParsePdu parse;

    char nBuffer[PACK_MAX_LEN] = {0};
    uint16_t nLength = PACK_MAX_LEN;
    if(!parse.packPduBuf(pdu_serial, nBuffer, nLength))
    {
        return 0;
    }
    return SendData(nBuffer, nLength, remote_ip, remote_port);

}
int UdpNetClient::SendData( char* data, uint32_t data_len,
              uint32_t remote_ip, uint16_t remote_port )
{
    if( NULL == socket_fd_ )
    {
        //LogERR("Send udp data error, invalid socket");
        return 0;
    }

    return net_event_loop_->SendData(socket_fd_, data, data_len, remote_ip, remote_port);
}

void UdpNetClient::GetLocalAddr( uint32_t &local_ip, uint16_t &local_port )
{
    if( NULL == socket_fd_ )
    {
        local_ip = 0;
        local_port = 0;
        return;
    }

    local_ip = local_ip_;
    local_port = local_port_;
}

void UdpNetClient::HandleSocketEvent(LPSOCKET socket, EM_SOCKET_EVENT ent, void* context)
{
    if (ent == EVENT_R)
    {
        char msg[UDP_BUF_SIZE];
        uint32_t remote_ip; uint16_t remote_port;
        int data_len = net_event_loop_->RecvData(socket, msg, UDP_BUF_SIZE, remote_ip, remote_port);
        if (0 > data_len)
        {
            int nError = 0;
            if (IsIgnoreSocketError_UDP(nError))
            {
                return;
            }
            Close();
            return;
        }
        remote_ip_ = remote_ip;
        remote_port_ = remote_port;
        uint16_t nType = 0;
        char *pData = NULL;
        uint16_t nDataLen = 0;
        uint8_t id = 0;
        ParsePdu parse;
        if (!parse.parsePduHead(msg, data_len, nType, id, pData, nDataLen))
        {
            return;
        }
        pdu_handler_->OnHandlePdu(nType, pData, nDataLen, remote_ip_, remote_port_, id);
    }
    else if (ent == EVENT_W)
    {
    }
}

void UdpNetClient::OnClosed(LPSOCKET socket_fd, void* context)
{
    running_ = false;

    ResetSocket();
	LogERR("Recv udp on closed event...");
}
