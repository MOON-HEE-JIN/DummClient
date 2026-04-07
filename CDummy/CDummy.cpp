#include "CDummy.h"
#include "../NetWork/CNetWork.h"
#include "DummyDef.h"
#include "../Log/CLog.h"
CDummy::CDummy()
{
	m_nReConnectTime = GetTickCount();
	m_nReConnectDelay = 5 * 1000;
	
	m_nLogTime = GetTickCount();
	m_nLogDelayTime = 5 * 1000;
}

CDummy::~CDummy()
{
	for (int i = 0; i < m_nMaxConnectClient; i++)
	{
		if (m_DummyClients[i] == nullptr)
			continue;
		delete m_DummyClients[i];
	}
}


void CDummy::Update()
{
	bool bReConnect = false;
	if (m_nReConnectTime + m_nReConnectDelay < GetTickCount())
	{
		m_nReConnectTime = GetTickCount();
		bReConnect = true;
	}

	for (int i = 0; i < m_nMaxConnectClient; ++i)
	{
		// Update logic can be added here if needed
		if (!m_DummyClients[i]->GetConnect())
		{
			if(!bReConnect)
				continue;

			m_DummyClients[i]->ReConnect("127.0.0.1", 7799, (HANDLE)GetCICPPort());

			m_DummyClients[i]->SendChangePidPacket();
			continue;
		}

		m_DummyClients[i]->LockSession();
		if (m_DummyClients[i]->IsSend())
			m_DummyClients[i]->SendPost();
		m_DummyClients[i]->UnLockSession();
	}

	if (m_nLogTime + m_nLogDelayTime < GetTickCount())
	{
		m_nLogTime = GetTickCount();
		float avg[MAX_ZONE_NUMBER+1] = { 0, };
		int cnt[MAX_ZONE_NUMBER+1] = { 0, };
		int nLoop = m_DummyClients.size();

		for (int i = 0; i < nLoop; i++)
		{
			float t = m_DummyClients[i]->GetAvgNetTime();
			if (t == 0)
				t = 0.01f;

			avg[m_DummyClients[i]->GetZoneID()] += t;
			cnt[m_DummyClients[i]->GetZoneID()]++;
		}

		g_LogDummy.ILog("Zone LatencyClients AvgLatency []::[]:[]");
		g_LogDummy.ILog("[1]::[%d]:[%.2f]\t[2]::[%d]:[%.2f]\t[3]::[%d]:[%.2f]"
			, cnt[1], avg[1], cnt[2], avg[2], cnt[3], avg[3]);
		g_LogDummy.ILog("[4]::[%d]:[%.2f]\t[5]::[%d]:[%.2f]\t[6]::[%d]:[%.2f]"
			, cnt[4], avg[4], cnt[5], avg[5], cnt[6], avg[6]);
	}
}

void CDummy::StartDummyClients()
{
	// 테스트 하는 클라 개수 = Zone 당 클라이언트 * 최대 Zone 개수
	m_nMaxConnectClient = MAX_CONNECT_CLIENT * MAX_ZONE_NUMBER;

	int ClinetID = 0;
	for (int i = 1; i <= MAX_ZONE_NUMBER; i++)
	{
		for (int j = 0; j < MAX_CONNECT_CLIENT; j++)
		{
			CClient* pClient = new CClient(ClinetID++, i, j);
			int ret = pClient->Connect("127.0.0.1", 7799, (HANDLE)GetCICPPort());
			if (ret != 0)
			{
				delete pClient;
				pClient = nullptr;
			}
			m_DummyClients.push_back(pClient);

			if(pClient != nullptr)
				pClient->SendChangePidPacket();
		}
	}
}


void CDummy::DisconnectClient(int id)
{
	m_nDiconnectClientCount++;
}

bool CDummy::RegisterServerIDtoClientID(int sID, int cID)
{
	if (m_DummyClientID.find(sID) != m_DummyClientID.end())
		return false;

	m_DummyClientID[sID] = cID;
	return true;
}

int CDummy::IsExistZoneClient(int zone, int sID)
{
	if (m_DummyClientID.find(sID) == m_DummyClientID.end())
		return DUMMY_ERROR::NOT_EXIST_CLIENT;

	int ClientID = m_DummyClientID[sID];
	for (int i = 0; i < m_nMaxConnectClient; i++)
	{
		if (ClientID != m_DummyClients[i]->GetClientID())
			continue;
		if (zone != m_DummyClients[i]->GetZoneID())
			return DUMMY_ERROR::NOT_EQUAL_ZONE;
	}

	return 0;
}

int CDummy::GetServerIDtoClientID(int sID)
{
	if (m_DummyClientID.find(sID) == m_DummyClientID.end())
		return -1;

	return m_DummyClientID[sID];
}

CClient* CDummy::GetClientByServerID(int sID)
{
	if (m_DummyClientID.find(sID) == m_DummyClientID.end())
		return nullptr;

	return m_DummyClients[m_DummyClientID[sID]];
}


CDummy g_DummyManager;