#ifndef __UDP_NETCLIENT_H__
#define __UDP_NETCLIENT_H__
#include "ParsePdu.h"
#include "NetClientBase.h"

class UdpNetClient : public NetClientBase, public INetUdpEvent
{
public:
    UdpNetClient(IUdpNetEventLoop* net_event_loop, PduHandler* pdu_handler);
    virtual ~UdpNetClient(void);

    //net client base impl
    virtual bool Open(uint32_t remote_ip = 0, uint16_t remote_port = 0, uint32_t local_ip = 0, uint16_t local_port = 0);
    virtual void Close();
    virtual bool IsClosed();
    virtual int SendRawData(char* data, uint32_t data_len);
    virtual int SendRawData(char* data, uint32_t data_len, uint32_t remote_ip, uint16_t remote_port);
    virtual int SendPduData(uint16_t cmd_id, char* data, uint32_t data_len);
    virtual int SendPduData(uint16_t cmd_id, char* data, uint32_t data_len, uint8_t id);
    virtual int SendPduData(uint16_t cmd_id, char* data, uint32_t data_len, uint32_t remote_ip, uint16_t remote_port);
    virtual int SendPduData(uint16_t cmd_id, char* data, uint32_t data_len, uint8_t id, uint32_t remote_ip, uint16_t remote_port);
    virtual void GetLocalAddr( uint32_t &local_ip, uint16_t &local_port );
    virtual bool SendPduData(PDUSerialBase& pdu_serial);
    virtual LPSOCKET GetSocketFd() { return socket_fd_; }
	virtual uint64_t GetSocketClosedTime() { return socket_closed_time_; };

    int SendData(char* data, uint32_t data_len,
        uint32_t remote_ip, uint16_t remote_port);
    int SendPduData(PDUSerialBase& pdu_serial, uint32_t remote_ip, uint16_t remote_port);

private:
    virtual void HandleSocketEvent(LPSOCKET socket, EM_SOCKET_EVENT ent, void* context);
    virtual void OnClosed(LPSOCKET socket_fd, void* context);
    void ResetSocket();

private:
    IUdpNetEventLoop* net_event_loop_;
    bool running_;
};
#endif

