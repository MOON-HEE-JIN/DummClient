#pragma once
#include "NetWork/CSession.h"
#include "MemoryManager/CLockFreeQueue_FromGPT.h"

#include "GameServerDef.h"
#include "Scheduling/ScheduleDefines.h"
#include "Scheduling/CSchedule.h"
#include "ClientDefines.h"
#include "CUtill/CLockQueue.h"

struct st_Schedule;

class CClient : public CSession
{
public:
	CClient(int Dummyid, int id);
	~CClient() {}

	// CSession 에서 상속
	void OnRecv(int type, CPacket& cPacket, double recvtime = 0) override;

	void DisConnect()
	{
		CloseSocket();
		Clear();
	}
private:
	int m_iManagementDummyID;		// 관리하는 CDummy ID
	int m_iID;
	int m_iServerID;
	
	int m_iSendDelay;				// 패킷 전송 지연 시간 (ms)
	int m_iSendTime;					// 패킷 전송 시간 (ms)

	bool m_bLogin;
	
	int m_iDefaultZoneID;				// 기본 Zone ID
	int m_iDefaultChannel;				// 기본 Channel
	int m_iZoneID;						// 현재 Zone ID
	int m_iChannel;						// 현재 Channel

	CLockQueue<RECV_JOB> m_queue;
	
	CSchedule* m_pSchedule;				// 현재 스케줄
	int m_iWorkScheduleLoop;
	int m_iWorkScheduleRogress;			// 현재 작업 스케줄 진행도
	st_Schedule* m_pWorkSchedule;		// 현재 작업 스케줄
	std::atomic<int> m_iCompleteScheduleCount;

	std::atomic<double> m_dLatencyTime;
private:
	ESTATE m_eState;
	st_Vector3F m_stPos;
	float m_fMoveSpeed;
	__int64 m_ddRecvLoopData;

public:
	void Init(int channel, int zone, CSchedule* pSchedule);
	void SetSchedule(CSchedule* pSchedule);
	int GetScheduleType();
private:
	void SetWorkSchedule(st_Schedule* pSchedule);
	void CheckSchedule();
	void SetFirstSchedule();
	void NextSchedule();
	
	void Clear();
public:
	void SetChangeZone(int channel, int zoneID) { m_iZoneID = zoneID; m_iChannel = channel; }
	void SetState(ESTATE state) { m_eState = state; }
	void SetRecvLoopBack(__int64 value) { m_ddRecvLoopData = value; }

	bool GetLogin() { return m_bLogin; }
	int GetID() { return m_iID; }
	int GetServerID() { return m_iServerID; }
	int GetDeafultZoneID() { return m_iDefaultZoneID; }
	int GetDefaultChannel() { return m_iDefaultChannel; }
	int GetZoneID() { return m_iZoneID; }
	int GetChannel() { return m_iChannel; }
	double GetLatency() { return m_dLatencyTime.load(); }
	float GetSpeed() { return m_fMoveSpeed; }
	int GetCompleteScheduleCount() { return m_iCompleteScheduleCount.load(); }
	__int64 GetRecvLoopBack() { return m_ddRecvLoopData; }
	const st_Schedule* GetWorkSchedule() { return m_pWorkSchedule; }
	st_Vector3F GetPos() { return m_stPos; }
	ESTATE GetState() { return m_eState; }

public:
	void AddCompleteScheduleCount() { m_iCompleteScheduleCount.fetch_add(1); }
	void AddPos(st_Vector3F pos) { m_stPos += pos; }
	void Arrive(st_Vector3F pos) { m_stPos = pos; }

public:
	void ConnectServerLoginThread(int id) { m_iServerID = id; m_bLogin = true; };	// 서버 로그인 thread 접속 완

	void Update();
};