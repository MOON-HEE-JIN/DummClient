#pragma once
#include "NetWork/CSession.h"
class CClient : public CSession
{
public:
	CClient(int ClientID);
	~CClient() {}

	// CSessionÀ»(¸¦) ÅëÇØ »ó¼ÓµÊ
	void OnRecv(int type, CPacket& cPacket) override;

	void SendChangePidPacket();
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

	int m_nServerProcID;
public:
	int GetSendReqDelayTime() const { return m_nSendReqDelayTime; }
	__int64 GetLoopbackData() const { return m_LoopbackData; }
	int GetServerProcID() { return m_nServerProcID; }

	__int64 IncrementLoopbackData() { return ++m_LoopbackData; }
	bool IncrementDisConnectRandomCount();

	bool IsSend();
};