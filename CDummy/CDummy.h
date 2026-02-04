#pragma once

#include "../CClient.h"
#include <vector>

class CDummy
{
public:
	CDummy();
	~CDummy();

	void Update();
	void StartDummyClients(int nClientCount);

	void SendLoopbackPackets();
	void DisconnectClient(int id);
private:
	std::vector<CClient*> m_DummyClients;
	int m_nDiconnectClientCount = 0;

	int m_nReConnectTime;
	int m_nReConnectDelay;
	int m_nMaxConnectClient;
};

extern CDummy g_DummyManager;