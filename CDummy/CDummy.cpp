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

	m_DummyClientByZone.resize(MAX_ZONE_NUMBER + 1);
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

		if (m_DummyClients[i]->IsSend())
			m_DummyClients[i]->SendPost();
	}

	if (m_nLogTime + m_nLogDelayTime < GetTickCount())
	{
		m_nLogTime = GetTickCount();
		for (int i = 1; i <= MAX_ZONE_NUMBER; i++)
		{
			DWORD total = 0;
			DWORD avgCnt = 0;
			for (int j = 0; j < MAX_CONNECT_CLIENT; j++)
			{
				float avg = m_DummyClientByZone[i][j]->GetAvgNetTime();
				if (avg == 0)
					avg = 0.01f;
				
				total += avg;
				avgCnt++;
			}

			g_LogDummy.ILog("Zone[%d] LatencyClients[%d] AvgLatency[%d]", i, avgCnt, total/avgCnt);
		}
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
			m_DummyClientByZone[i].push_back(pClient);
			if(pClient != nullptr)
				pClient->SendChangePidPacket();
		}
	}
}

void CDummy::SendLoopbackPackets()
{
	for (CClient* pClient : m_DummyClients)
	{
		pClient->SendChangePidPacket();
		pClient->SendLoopbackPacket();
	}
}

void CDummy::DisconnectClient(int id)
{
	m_nDiconnectClientCount++;
}


CDummy g_DummyManager;