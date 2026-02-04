#include "CDummy.h"
#include "../NetWork/CNetWork.h"
CDummy::CDummy()
{
}

CDummy::~CDummy()
{
}


void CDummy::Update()
{
	for(CClient* pClient : m_DummyClients)
	{
		// Update logic can be added here if needed
		if (pClient->IsSend())
			pClient->SendPost();
	}
}

void CDummy::StartDummyClients(int nClientCount)
{

	for (int i = 0; i < nClientCount; ++i)
	{
		CClient* pClient = new CClient;
		pClient->Connect("127.0.0.1", 7799, (HANDLE)GetCICPPort());
	}
}

void CDummy::SendLoopbackPackets()
{
	for (CClient* pClient : m_DummyClients)
	{
		pClient->SendLoopbackPacket();
	}
}
