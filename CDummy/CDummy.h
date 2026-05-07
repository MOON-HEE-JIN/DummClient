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
public:
	const std::vector<CClient*>& GetDummyClients() { return m_DummyClients; };

private:
	void CreateDummyClient();
	void ReleaseDummyClient();
	void Update();

	static unsigned __stdcall RunThread(void* arg);		// accept() Thread
	int Run();
public:
	void Init(int channel, int zone, CSchedule* pSchedule);

	void Start();
	void Wait();
	void Stop();
};