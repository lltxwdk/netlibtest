#ifndef __TCP_NETCLIENT_H__
#define __TCP_NETCLIENT_H__

#include "TcpParsePdu.h"
#include "NetClientBase.h"
class TcpNetClient : public NetClientBase, public INetTcpEvent
{
public:
    TcpNetClient(ITcpNetEventLoop* net_event_loop, PduHandler* pdu_handler);
    TcpNetClient(ITcpNetEventLoop* net_event_loop, PduHandler* pdu_handler,
        uint32_t max_sendbuf_len, uint32_t max_recvbuf_len);
    virtual ~TcpNetClient(void);

    //net client base impl
    virtual bool Open(uint32_t remote_ip, uint16_t remote_port, uint32_t local_ip = 0, uint16_t local_port = 0); //open for connecting
    bool Open(LPSOCKET socket, uint32_t remote_ip, uint16_t remote_port);//open for accepting
    virtual void Close();
    virtual bool IsClosed();
    virtual int SendRawData(char* data, uint32_t data_len);
    virtual int SendPduData(uint16_t cmd_id, char* data, uint32_t data_len);
    virtual int SendPduData(uint16_t cmd_id, char* data, uint32_t data_len, uint8_t id);
    void GetLocalAddr( uint32_t &local_ip, uint16_t &local_port );
    virtual bool SendPduData(PDUSerialBase& pdu_serial);
    virtual LPSOCKET GetSocketFd() { return socket_fd_; }
    virtual uint64_t GetSocketClosedTime() { return socket_closed_time_; };

    //tcpparsepud callback
    void OnHandleTcpPdu(uint16_t cmd_id, char* pdu_data, uint32_t data_len);
    int Send(char* data, uint32_t data_len);
    void PeerClose();

private:
    //implement INetTcpEvent
    virtual void HandleSocketEvent(LPSOCKET socket, EM_SOCKET_EVENT ent, void* context);
    //virtual void OnSendReady(LPSOCKET socket_fd, void* context);
    virtual void OnClosed(LPSOCKET socket_fd, void* context);
    virtual void OnConnected(LPSOCKET socket_fd, void* context);
    virtual bool IsNeedSend(LPSOCKET socket_fd, void* context);

    void ResetSocket(bool bActive);

private:
    ITcpNetEventLoop *tcp_event_loop_;
    TcpParsePdu pdu_parser_;
    bool m_bConnected;
};

#endif

