#pragma once
#include "NetWork/CSession.h"
class CClient : public CSession
{
public:
	CClient();
	~CClient() {}

	// CSession을(를) 통해 상속됨
	void OnRecv(int type, CPacket& cPacket) override;

	void SendLoopbackPacket();
private:
	__int64 m_LoopbackData;
	int m_nSendReqDelayTime;
	int m_nLastSendReqTime;
public:
	int GetSendReqDelayTime() const { return m_nSendReqDelayTime; }
	__int64 GetLoopbackData() const { return m_LoopbackData; }

	__int64 IncrementLoopbackData() { return ++m_LoopbackData; }
	bool IsSend();
};