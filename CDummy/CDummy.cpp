#include "CDummy.h"
#include "../NetWork/CNetWork.h"
#include "DummyDef.h"
#include "../Log/CLog.h"

#include <process.h>

#include "../Test/TSchedule_Change_Zone.h"

CDummy::CDummy(int id, const char* ip, short port, int maxDummyClientCount, int defaultid)
{
	m_iID = id;
	memcpy(m_szIP, ip, sizeof(m_szIP));
	m_sPort = port;
	m_iMaxDummyClientCount = maxDummyClientCount;

	m_bRun = false;
	m_hThread = 0;
	m_hExitEvent = 0;

	m_pSchedule = nullptr;

	m_iDummyDefaultID = defaultid;

	m_iDelayLatencyTime = 3 * 1000;
	m_iLatencyTime = 0;
}

CDummy::~CDummy()
{
	ReleaseDummyClient();
}


void CDummy::Update()
{
	for (int i = 0; i < m_iMaxDummyClientCount; i++)
	{
		if (m_DummyClients[i] == nullptr)
			continue;

		m_DummyClients[i]->Update();
	}

	LogClientLatencyTime();
}

void CDummy::CreateDummyClient()
{
	int max = m_iMaxDummyClientCount + m_iDummyDefaultID;
	for (int i = m_iDummyDefaultID; i < max; i++)
	{
		CClient* pClient = new CClient(m_iID, i);
		int ret = pClient->Connect(m_szIP, m_sPort, (HANDLE)GetCICPPort());
		if (ret != 0)
		{
			delete pClient;
			pClient = nullptr;
			g_LogDummy.ELog("ERROR Create Client");
			exit(1);
		}
		pClient->Init(m_iDummyChannel, m_iDummyZone, m_pSchedule);
		m_DummyClients.push_back(pClient);
	}
}

void CDummy::ReleaseDummyClient()
{
	for (int i = 0; i < m_iMaxDummyClientCount; i++)
	{
		if (m_DummyClients[i] == nullptr)
			continue;
		delete m_DummyClients[i];
	}
	m_DummyClients.clear();
}

void CDummy::LogClientLatencyTime()
{
	if (m_iLatencyTime + m_iDelayLatencyTime < GetTickCount())
	{
		double value = m_DummyClients[0]->GetLatency();
		double maxTime = value;
		double minTime = value;
		double totalTime = value;

		for (int i = 1; i < m_iMaxDummyClientCount; i++)
		{
			value = m_DummyClients[i]->GetLatency();
			maxTime = max(maxTime, value);
			minTime = min(minTime, value);
			totalTime += value;
		}
		
		double avg = totalTime / m_iMaxDummyClientCount;

		g_LogDummy.ILog("DummyID[%d] Max : %.3f, Min : %.3f, Avg : %.3f", m_iID, maxTime, minTime, avg);

		m_iLatencyTime = GetTickCount();
	}
}

void CDummy::Init(int channel, int zone, CSchedule* pSchedule)
{
	m_iDummyChannel = channel;
	m_iDummyZone = zone;
	m_pSchedule = pSchedule;

	CreateDummyClient();
}
