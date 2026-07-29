#pragma once

#include "CDummy.h"
#include "../Scheduling/CSchedule.h"
#include <map>
#include <atomic>

#define THREAD_CLIENT_COUNT 1000  //thread 당 클라 담당 개수

struct st_ThreadLock
{
	std::atomic<bool> bChange;
	CRITICAL_SECTION cs;

	st_ThreadLock()
	{
		bChange = false;
		InitializeCriticalSection(&cs);
	}

	~st_ThreadLock()
	{
		DeleteCriticalSection(&cs);
	}

	void Lock()
	{
		EnterCriticalSection(&cs);
	}

	void UnLock()
	{
		LeaveCriticalSection(&cs);
	}
};

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
	std::atomic<bool> m_bRun = true;

	int m_iDelayLatencyTime = 1 * 1000;
	std::atomic<ULONGLONG> m_iLatencyTime = 0;

	std::vector<CSchedule*> m_vecSchedules;

	CRITICAL_SECTION cs;
public:
	bool CreateDummy(int channel, int zone, int count, int scheduleType);
	void AddDummyClient(CClient* pClient);
	CSchedule* GetSchedule(int index)
	{
		if (index < 0 || index >= static_cast<int>(m_vecSchedules.size()))
			return nullptr;
		return m_vecSchedules[index];
	}

private:
	HANDLE m_hExit;
	std::vector<HANDLE> m_vecDummyThreadHandles;
	std::vector<int> m_vecThreadDummyClientCount;
	std::vector<st_ThreadLock*> m_vecThreadLock;
	std::map<int, std::vector<CDummy*>> m_mapThreadDummy;

	static unsigned __stdcall RunThread(void* arg);
	void Run(const int id);

	void RegisterThread(CDummy* pDummy);
};

extern CDummyManager g_DummyManager;
