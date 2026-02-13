#pragma once

#include "../CClient.h"
#include <vector>
#include <cstdint>
#include <atomic>

class CDummy
{
public:
	CDummy();
	~CDummy();

	void Update();
	void StartDummyClients(int nClientCount);

	void SendLoopbackPackets();
	void DisconnectClient(int id);

	void NotifyClientDisconnected(int id, bool bForced);
	void NotifyLoopbackLatency(int id, int lastLatencyMs, int avgLatencyMs);
	void NotifyLoopbackDataError(int id, __int64 recvData, __int64 expectedData);
	void NotifyChangePidError(int id, int errorCode);
private:
	std::vector<CClient*> m_DummyClients;
	int m_nDiconnectClientCount = 0;

	int m_nReConnectDelay;
	int m_nMaxConnectClient;
	std::vector<DWORD> m_ReConnectTick;

	DWORD m_nLastLogTick;
	int m_nLogIntervalMs;

	std::atomic<int> m_nReconnectCount;
	std::atomic<int> m_nDisconnectForcedCount;
	std::atomic<int> m_nDisconnectRandomCount;
	std::atomic<int> m_nLoopbackSampleCount;
	std::atomic<long long> m_nLoopbackLastLatencySum;
	std::atomic<long long> m_nLoopbackAvgLatencySum;
	std::atomic<int> m_nLoopbackDataErrorCount;
	std::atomic<int> m_nChangePidErrorCount;
};

extern CDummy g_DummyManager;
