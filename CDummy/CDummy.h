#pragma once

#include "../CClient.h"
#include <vector>

class CDummy
{
public:
	CDummy();
	~CDummy();

	void Update();
	void StartDummyClients();

	void SendLoopbackPackets();
	void DisconnectClient(int id);
private:
	std::vector<CClient*> m_DummyClients;							// Client 전체 관리
	std::vector<std::vector<CClient*>> m_DummyClientByZone;			// Client Zone 구분 관리

	int m_nDiconnectClientCount = 0;

	int m_nReConnectTime;
	int m_nReConnectDelay;
	int m_nMaxConnectClient;

	int m_nLogTime;
	int m_nLogDelayTime;
};

extern CDummy g_DummyManager;