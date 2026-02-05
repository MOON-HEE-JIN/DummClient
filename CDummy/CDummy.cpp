#include "CDummy.h"
#include "../NetWork/CNetWork.h"
CDummy::CDummy()
{
	m_nReConnectTime = GetTickCount();
	m_nReConnectDelay = 5 * 1000;
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
		bReConnect = true;

	for (int i = 0; i < m_nMaxConnectClient; ++i)
	{
		// Update logic can be added here if needed
		if (m_DummyClients[i] == nullptr)
		{
			if(!bReConnect)
				continue;

			CClient* pClient = new CClient(i);
			printf("ReConnect Client %d\n", i);
			pClient->Connect("127.0.0.1", 7799, (HANDLE)GetCICPPort());
			pClient->SendLoopbackPacket();
			m_DummyClients[i] = pClient;
			continue;
		}
		if (m_DummyClients[i]->IsSend())
			m_DummyClients[i]->SendPost();
	}

}

void CDummy::StartDummyClients(int nClientCount)
{
	m_nMaxConnectClient = nClientCount;
	for (int i = 0; i < nClientCount; ++i)
	{
		CClient* pClient = new CClient(i);
		pClient->Connect("127.0.0.1", 7799, (HANDLE)GetCICPPort());
		m_DummyClients.push_back(pClient);
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
	m_DummyClients[id] = nullptr;
	m_nDiconnectClientCount++;
	printf("Disconnect Client ID : %d, TotalDisConnectCount : %d\n", id, m_nDiconnectClientCount);
}


CDummy g_DummyManager;