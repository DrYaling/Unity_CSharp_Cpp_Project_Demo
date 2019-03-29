#ifndef KCP_CLIENT_H
#define KCP_CLIENT_H
#include "kcp_config.h"
#include "SocketConfig.h"
#include "CSocket.h"

class KcpClient
{
public:
	KcpClient(int16 sid);
	~KcpClient();
	void SetAddress(const char* ip,int16 port);
	void Connect();
	void Close();
	void Send(char* buff, int length, bool immediately = false);
	void OnReceive(const uint8* buff, int length);
	void Update(int32_t time);
	bool IsAlive() { return m_nSessionStatus == SessionStatus::Connected; }
	bool IsClosed() { return m_nSessionStatus == SessionStatus::Disconnected; }
	bool IsConnecting() { return m_nSessionStatus == SessionStatus::Connecting; }
	bool IsConnected() { return m_nSessionStatus == SessionStatus::Connected; }
public:
	void SetReceiveCallBack(SocketDataReceiveHandler cb) { m_pDataHandler = cb; }
private:
	void ReadHandlerInternal(int size, const char * buffer);
	ReadDataHandlerResult ReadDataHandler();
	bool ReadHeaderHandler();
	void ReadHandler();
	void OnDisconnected(bool immediately);
	void OnConnected(bool success);
	bool CheckCrc(const char* buff, int length) { return true; }
	static int fnWriteDgram(const char *buf, int len, ikcpcb *kcp, void *user);
	void SendHeartBeat();
private:
	SocketDataReceiveHandler m_pDataHandler;
	MessageBuffer m_headerBuffer;
	MessageBuffer m_packetBuffer;
	MessageBuffer m_readBuffer;
	SockAddr_t m_stremote;
	socketSessionId m_stSessionId;
	ikcpcb* m_pKcp;
	Socket* m_pSocket;
	int32 m_nSessionStatus;
	int32 m_nTick;
	uint64 m_nLastSendTime;
	uint64 m_nNeedUpdateTime;
	bool m_bNeedUpdate;
	bool m_bAlive;
	int16 m_nServerId;
	bool m_bRecv;

};
#endif