#pragma once

#include "../CClient.h"
#include <vector>
#include <unordered_map>

class CDummy
{
public:
	CDummy();
	~CDummy();

	void Update();
	void StartDummyClients();

	void DisconnectClient(int id);
	bool RegisterServerIDtoClientID(int sID, int cID);
	int IsExistZoneClient(int zone, int sID);
private:
	std::vector<CClient*> m_DummyClients;							// Client 전체 관리
	std::unordered_map<int, int> m_DummyClientID;					// ServerID - ClientID;
	int m_nDiconnectClientCount = 0;

	int m_nReConnectTime;
	int m_nReConnectDelay;
	int m_nMaxConnectClient;

	int m_nLogTime;
	int m_nLogDelayTime;

public:
	int GetServerIDtoClientID(int sID);
	CClient* GetClientByServerID(int sID);
};

extern CDummy g_DummyManager;