#pragma once
#include "NetWork/CSession.h"
class CClient : public CSession
{
public:
	CClient(int ClientID, int testZone, int inZoneClientID);
	~CClient() {}

	// CSession 에서 상속
	void OnRecv(int type, CPacket& cPacket) override;

	void DisConnect()
	{
		CloseSocket();
	}
	void ReConnect(const char IP[16], unsigned short Port, HANDLE cicp);

private:
	int m_nClientID;		// 클라이언트 구분 ID
	int m_nZoneID;		// 클라이언트 Zone ID
	int m_nServerClientID;	// Server 에서 사용중인 ID

	st_Vector3F m_stPosition;
	st_Vector3F m_stDirection;
	st_Vector3F m_stGoalPosition;
	float m_fSpeed;

private:
	int m_iChannel;
	int m_nPreZoneID;
	int m_nInZoneClientID;	// Dummy 에서 관리하는 Zone 내에서의 자기 vector index
	
	std::atomic<bool> m_bLogin = false;
	std::atomic<bool> m_bWaitServerResponse;		// 서버 응답을 기다리는 상태

	__int64 m_LoopbackData;
	int m_nSendReqDelayTime;
	int m_nLastSendReqTime;

	int m_nLoopbackSendCount;
	int m_nDisConnectRandomCount;
	
	int m_iChangeZoneDelayTime = 3 * 1000;
	int m_iChangeZoneTime = 0;

	double m_dMoveStartTime;

	bool m_bLoopBack;

	int m_iStartChannel;
	int m_iStartZone;
	int m_iEndChannel;
	int m_iEndZone;
public:
	int GetSendReqDelayTime() const { return m_nSendReqDelayTime; }
	__int64 GetLoopbackData() const { return m_LoopbackData; }
	int GetChannel() { return m_iChannel; }
	int GetZoneID() { return m_nZoneID; }
	int	GetClientID() { return m_nClientID; }
	int GetServerClientID() { return m_nServerClientID; }
	double GetMoveStartTime() const { return m_dMoveStartTime; }
	st_Vector3F GetPosition() const { return m_stPosition; }
	st_Vector3F GetDirection() const { return m_stDirection; }
	st_Vector3F GetGoalPosition() const { return m_stGoalPosition; }
	bool GetLoopBack() { return m_bLoopBack; }
	bool GetLogin() { return m_bLogin.load(); }

	void CreateCharInfo(st_Vector3F pos, float speed);
	void SetServerClientID(int value);

	void SetLogin() { m_bLogin.store(true); }

	void WaitLoginThreadResponse() { m_bWaitServerResponse = true; }	// game 서버에서 login thread 에 들어가기 전까지는 보내지 않기
	void CompletedWaitServerResponse() { m_bWaitServerResponse = false; }

	__int64 IncrementLoopbackData() { return ++m_LoopbackData; }
	bool IncrementDisConnectRandomCount();
	void ReStoreZoneID() { m_nZoneID = m_nPreZoneID; };
	bool IsSend();
	void MoveStop(st_Vector3F comparevector);
	void SetChannel(int channel) { m_iChannel = channel; };
	void SetZoneID(int zone) { m_nZoneID = zone; }
	void SetLoopBack(bool b) { m_bLoopBack = b; }

	void SetChangeZoneVar();
public:

};