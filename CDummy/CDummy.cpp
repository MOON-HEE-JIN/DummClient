#include "CDummy.h"
#include "../NetWork/CNetWork.h"
#include "DummyDef.h"
#include "../Log/CLog.h"

#include <process.h>

#include "../Test/TSchedule_Change_Zone.h"

CDummy::CDummy(int id, const char* ip, short port, int maxDummyClientCount, int defaultid)
{
	m_iID = id;
	strncpy_s(m_szIP, ip, _TRUNCATE);
	m_sPort = port;
	m_iMaxDummyClientCount = maxDummyClientCount;
	m_iDummyChannel = 0;
	m_iDummyZone = 0;

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
	for (CClient* client : m_DummyClients)
	{
		if (client == nullptr)
			continue;

		client->Update();
	}

	LogClientLatencyTime();
}

bool CDummy::CreateDummyClient()
{
	int max = m_iMaxDummyClientCount + m_iDummyDefaultID;
	for (int i = m_iDummyDefaultID; i < max; i++)
	{
		CClient* pClient = new CClient(m_iID, i);
		int ret = pClient->Connect(m_szIP, m_sPort, (HANDLE)GetCICPPort());
		if (ret != 0)
		{
			delete pClient;
			g_LogDummy.ELog("ERROR Create Client[%d] WSA[%d]", i, ret);
			ReleaseDummyClient();
			return false;
		}
		pClient->Init(m_iDummyChannel, m_iDummyZone, m_pSchedule);
		
		// SendDelayTime 을 일관되서 맞춰서 테스트
		if (m_iConsistentTime > 0)
			pClient->SetConsistentTime(m_iConsistentTime);

		m_DummyClients.push_back(pClient);
	}

	return true;
}

void CDummy::ReleaseDummyClient()
{
	for (CClient* client : m_DummyClients)
	{
		delete client;
	}
	m_DummyClients.clear();
}

void CDummy::LogClientLatencyTime()
{
	if (m_DummyClients.empty())
		return;

	const ULONGLONG now = GetTickCount64();
	if (m_iLatencyTime + static_cast<ULONGLONG>(m_iDelayLatencyTime) < now)
	{
		double value = m_DummyClients[0]->GetLatency();
		m_ddMaxTime = value;
		m_ddMinTime = value;
		double totalTime = value;

		m_iMaxCompleteScheduleCount = m_DummyClients[0]->GetCompleteScheduleCount();
		m_iMinCompleteScheduleCount = m_DummyClients[0]->GetCompleteScheduleCount();

		for (size_t i = 1; i < m_DummyClients.size(); i++)
		{
			value = m_DummyClients[i]->GetLatency();
			m_ddMaxTime = max(m_ddMaxTime, value);
			m_ddMinTime = min(m_ddMinTime, value);

			m_iMaxCompleteScheduleCount = max(m_iMaxCompleteScheduleCount, m_DummyClients[i]->GetCompleteScheduleCount());
			m_iMinCompleteScheduleCount = min(m_iMinCompleteScheduleCount, m_DummyClients[i]->GetCompleteScheduleCount());

			totalTime += value;
		}
		
		m_ddAvgTime = totalTime / static_cast<double>(m_DummyClients.size());

		m_iLatencyTime = now;
	}
}

bool CDummy::Init(int channel, int zone, CSchedule* pSchedule)
{
	m_iDummyChannel = channel;
	m_iDummyZone = zone;
	m_pSchedule = pSchedule;
	if (pSchedule->GetType() == ESCHEDULE_TEST_TYPE::SCHEDULE_LOOPBACK_DISRECONNECT)
	{
		m_iConsistentTime = 5 * 100;
	}
	return CreateDummyClient();
}
