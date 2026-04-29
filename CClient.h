#pragma once
#include "NetWork/CSession.h"
#include "MemoryManager/CLockFreeQueue_FromGPT.h"

#include "GameServerDef.h"
#include "Scheduling/ScheduleDefines.h"
#include "Scheduling/CSchedule.h"

struct st_Schedule;

class CClient : public CSession
{
public:
	CClient(int Dummyid, int id);
	~CClient() {}

	// CSession 에서 상속
	void OnRecv(int type, CPacket& cPacket) override;

	void DisConnect()
	{
		CloseSocket();
		Clear();
	}
private:
	int m_iManagementDummyID;		// 관리하는 CDummy ID
	int m_iID;
	int m_iServerID;

	bool m_bLogin;
	
	int m_iDefaultZoneID;				// 기본 Zone ID
	int m_iDefaultChannel;				// 기본 Channel
	int m_iZoneID;						// 현재 Zone ID
	int m_iChannel;						// 현재 Channel

	CLockFreeQueue_MPSC<RECV_JOB> m_PacketPool;

	CSchedule* m_pSchedule;				// 현재 스케줄
	int m_iWorkScheduleRogress;			// 현재 작업 스케줄 진행도
	st_Schedule* m_pWorkSchedule;		// 현재 작업 스케줄
private:
	void SetSchedule(CSchedule* pSchedule);
	void SetWorkSchedule(st_Schedule* pSchedule) { m_pWorkSchedule = pSchedule; }
	void CheckSchedule();
	void NextSchedule();
	
	void Clear();
public:
	bool GetLogin() { return m_bLogin; }
	int GetDeafultZoneID() { return m_iDefaultZoneID; }
	int GetDefaultChannel() { return m_iDefaultChannel; }
	int GetZoneID() { return m_iZoneID; }
	int GetChannel() { return m_iChannel; }

public:
	void ConnectServerLoginThread(int id) { m_iServerID = id;};	// 서버 로그인 thread 접속 완

	void Update();
};