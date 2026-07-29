#pragma once

#include "../CClient.h"
#include <vector>
#include <unordered_map>

#include "../Scheduling/ScheduleDefines.h"
#include "../Scheduling/CSchedule.h"
class CDummy
{
public:
	CDummy(int id, const char* ip, short port, int maxDummyClientCount, int defaultid);
	~CDummy();

private:
	int m_iID;
	char m_szIP[16];
	short m_sPort;
	bool m_bRun;
	int m_iMaxDummyClientCount;

	int m_iDummyChannel;
	int m_iDummyZone;
	int m_iDummyDefaultID;
	CSchedule* m_pSchedule;

	std::vector<CClient*> m_DummyClients;							// Client 전체 관리
	
	HANDLE m_hThread;
	HANDLE m_hExitEvent;
private:
	double m_ddMaxTime = 0;
	double m_ddMinTime = 0;
	double m_ddAvgTime = 0;
	int m_iMaxCompleteScheduleCount = 0;
	int m_iMinCompleteScheduleCount = 0;
public:
	double GetMaxTime() { return m_ddMaxTime; }
	double GetMinTime() { return m_ddMinTime; }
	double GetAvgTime() { return m_ddAvgTime; }
	int GetMaxComplete() { return m_iMaxCompleteScheduleCount; }
	int GetMinComplete() { return m_iMinCompleteScheduleCount; }

public:
	const std::vector<CClient*>& GetDummyClients() { return m_DummyClients; };

	int m_iDelayLatencyTime;
	ULONGLONG m_iLatencyTime;

private:
	bool CreateDummyClient();
	void ReleaseDummyClient();
	void LogClientLatencyTime();

public:
	bool Init(int channel, int zone, CSchedule* pSchedule);
	void Update();
};
