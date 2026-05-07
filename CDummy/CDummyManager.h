#pragma once

#include "CDummy.h"
#include "../Scheduling/CSchedule.h"
#include <map>

class CDummyManager
{
public:
	CDummyManager();
	~CDummyManager();

private:
	std::map<int, CDummy*> m_mapDummys;
	std::map<int, CClient*> m_mapDummyClients;

	int m_iDummyID;
	int m_iClientID;
	char m_szIP[16];
	short m_sPort;
	
	std::vector<CSchedule*> m_vecSchedules;

	CRITICAL_SECTION cs;
public:
	bool CreateDummy(int channel, int zone, int count, int scheduleType);
	void AddDummyClient(CClient* pClient);
	void ReleaseDummy(int dummyID);
};

extern CDummyManager g_DummyManager;