#pragma once
#include "NetWork/CSession.h"
class CClient : public CSession
{
public:
	CClient(int ClientID);
	~CClient() {}

	// CSession을(를) 통해 상속됨
	void OnRecv(int type, CPacket& cPacket) override;

	void SendLoopbackPacket();


	void DisConnect()
	{
		CloseSocket();
	}
	void ReConnect(const char IP[16], unsigned short Port, HANDLE cicp)
	{
		Connect(IP, Port, cicp);
	}

private:
	int m_nClientID;

	__int64 m_LoopbackData;
	int m_nSendReqDelayTime;
	int m_nLastSendReqTime;

	int m_nDisConnectRandomCount;
	int m_nLoopbackSendCount;

public:
	int GetSendReqDelayTime() const { return m_nSendReqDelayTime; }
	__int64 GetLoopbackData() const { return m_LoopbackData; }

	__int64 IncrementLoopbackData() { return ++m_LoopbackData; }
	bool IncrementDisConnectRandomCount();

	bool IsSend();
};