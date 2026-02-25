#pragma once
#include "NetWork/CSession.h"
class CClient : public CSession
{
public:
	CClient(int ClientID, int testZone, int inZoneClientID);
	~CClient() {}

	// CSession 에서 상속
	void OnRecv(int type, CPacket& cPacket) override;

	void SendChangePidPacket();
	void SendLoopbackPacket();


	void DisConnect()
	{
		CloseSocket();
	}
	void ReConnect(const char IP[16], unsigned short Port, HANDLE cicp);

private:
	int m_nClientID;		// 클라이언트 구분 ID
	int m_nZoneID;		// 클라이언트 Zone ID
	int m_nPreZoneID;
	int m_nInZoneClientID;	// Dummy 에서 관리하는 Zone 내에서의 자기 vector index
	int m_nServerClientID;	// Server 에서 사용중인 ID

	std::atomic<bool> m_bWaitServerResponse;		// 서버 응답을 기다리는 상태

	__int64 m_LoopbackData;
	int m_nSendReqDelayTime;
	int m_nLastSendReqTime;

	int m_nLoopbackSendCount;
	int m_nDisConnectRandomCount;
	
	int m_nChangeZoneIDCount;
	int m_nChangeZoneIDRequestCount;

	int m_nServerProcID;

	st_Vector m_stPosition;
public:
	int GetSendReqDelayTime() const { return m_nSendReqDelayTime; }
	__int64 GetLoopbackData() const { return m_LoopbackData; }
	int GetZoneID() { return m_nZoneID; }
	int	GetClientID() { return m_nClientID; }
	int GetServerClientID() { return m_nServerClientID; }

	void CreateCharInfo(st_Vector pos);
	void SetServerClientID(int value);

	void CompletedWaitServerResponse() { m_bWaitServerResponse = false; }

	__int64 IncrementLoopbackData() { return ++m_LoopbackData; }
	bool IncrementDisConnectRandomCount();
	void ChangeZoneIDRequest();
	void ReStoreZoneID() { m_nZoneID = m_nPreZoneID; };
	bool IsSend();
};