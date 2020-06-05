#ifndef __NET_CLIENT_BASE_H__
#define __NET_CLIENT_BASE_H__
#include "ParsePdu.h"
#include "NetworkInterface.h"

class NetClientBase
{
public:
    NetClientBase(PduHandler* pdu_handler)
        :pdu_handler_(pdu_handler)
    {
        socket_fd_ = NULL;
        remote_ip_ = 0;
        remote_port_ = 0;
        local_ip_ = 0;
        local_port_ = 0;
		socket_closed_time_ = 0;
    }
    virtual ~NetClientBase(void){}
    virtual bool Open(uint32_t remote_ip = 0, uint16_t remote_port = 0, uint32_t local_ip = 0, uint16_t local_port = 0) = 0;
    virtual void Close() = 0;
    virtual bool IsClosed() = 0;
    virtual int SendRawData(char* data, uint32_t data_len) = 0;
    virtual int SendPduData(uint16_t cmd_id, char* data, uint32_t data_len) = 0;
    virtual int SendPduData(uint16_t cmd_id, char* data, uint32_t data_len, uint8_t id) = 0;
    virtual void GetLocalAddr( uint32_t &local_ip, uint16_t &local_port ) = 0;
    virtual LPSOCKET GetSocketFd() = 0;
    virtual bool SendPduData(PDUSerialBase& pdu_serial) = 0;
    virtual uint64_t GetSocketClosedTime() = 0;

protected:
    PduHandler* pdu_handler_;
    LPSOCKET socket_fd_;
    uint32_t remote_ip_;
    uint16_t remote_port_;
    uint32_t local_ip_;
    uint16_t local_port_;
    uint64_t socket_closed_time_;
};

#endif


