#include "CDummy.h"
#include "../NetWork/CNetWork.h"

CDummy::CDummy()
{
	m_nReConnectDelay = 5 * 1000;
	m_nMaxConnectClient = 0;

	m_nLastLogTick = GetTickCount();
	m_nLogIntervalMs = 3 * 1000;

	m_nReconnectCount = 0;
	m_nDisconnectForcedCount = 0;
	m_nDisconnectRandomCount = 0;
	m_nLoopbackSampleCount = 0;
	m_nLoopbackLastLatencySum = 0;
	m_nLoopbackAvgLatencySum = 0;
	m_nLoopbackDataErrorCount = 0;
	m_nChangePidErrorCount = 0;
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
	const DWORD now = GetTickCount();
	for (int i = 0; i < m_nMaxConnectClient; ++i)
	{
		if (m_DummyClients[i] == nullptr)
		{
			if (m_ReConnectTick[i] == 0 || m_ReConnectTick[i] > now)
				continue;

			CClient* pClient = new CClient(i);
			pClient->Connect("127.0.0.1", 7799, (HANDLE)GetCICPPort());
			pClient->SendChangePidPacket();
			pClient->SendPost();
			m_DummyClients[i] = pClient;
			m_ReConnectTick[i] = 0;
			m_nReconnectCount.fetch_add(1);
			continue;
		}

		if (m_DummyClients[i]->IsSend())
			m_DummyClients[i]->SendPost();
	}

	if (m_nLastLogTick + m_nLogIntervalMs < now)
	{
		m_nLastLogTick = now;

		int connectedCount = 0;
		for (int i = 0; i < m_nMaxConnectClient; ++i)
		{
			if (m_DummyClients[i] != nullptr)
				connectedCount++;
		}

		const int reconnectCount = m_nReconnectCount.exchange(0);
		const int disconnectForcedCount = m_nDisconnectForcedCount.exchange(0);
		const int disconnectRandomCount = m_nDisconnectRandomCount.exchange(0);
		const int loopbackSampleCount = m_nLoopbackSampleCount.exchange(0);
		const long long loopbackLastLatencySum = m_nLoopbackLastLatencySum.exchange(0);
		const long long loopbackAvgLatencySum = m_nLoopbackAvgLatencySum.exchange(0);
		const int loopbackDataErrorCount = m_nLoopbackDataErrorCount.exchange(0);
		const int changePidErrorCount = m_nChangePidErrorCount.exchange(0);

		const int avgLastLatency = (loopbackSampleCount > 0)
			? static_cast<int>(loopbackLastLatencySum / loopbackSampleCount) : 0;
		const int avgLatency = (loopbackSampleCount > 0)
			? static_cast<int>(loopbackAvgLatencySum / loopbackSampleCount) : 0;

		printf("[DummySummary/3s] clients=%d connected=%d reconnect=%d disconnect(total=%d,forced=%d,random=%d) "
			"latency(ms avg:last=%d,avg=%d,samples=%d) errors(loopback=%d,changepid=%d)\n",
			m_nMaxConnectClient,
			connectedCount,
			reconnectCount,
			m_nDiconnectClientCount,
			disconnectForcedCount,
			disconnectRandomCount,
			avgLastLatency,
			avgLatency,
			loopbackSampleCount,
			loopbackDataErrorCount,
			changePidErrorCount);
	}

}

void CDummy::StartDummyClients(int nClientCount)
{
	m_nMaxConnectClient = nClientCount;
	m_ReConnectTick.assign(nClientCount, 0);
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
		if (pClient == nullptr)
			continue;

		pClient->SendChangePidPacket();
		pClient->SendPost();
	}
}

void CDummy::DisconnectClient(int id)
{
	m_DummyClients[id] = nullptr;
	m_ReConnectTick[id] = GetTickCount() + m_nReConnectDelay;
	m_nDiconnectClientCount++;
}

void CDummy::NotifyClientDisconnected(int id, bool bForced)
{
	(void)id;
	if (bForced)
		m_nDisconnectForcedCount.fetch_add(1);
	else
		m_nDisconnectRandomCount.fetch_add(1);
}

void CDummy::NotifyLoopbackLatency(int id, int lastLatencyMs, int avgLatencyMs)
{
	(void)id;
	m_nLoopbackSampleCount.fetch_add(1);
	m_nLoopbackLastLatencySum.fetch_add(lastLatencyMs);
	m_nLoopbackAvgLatencySum.fetch_add(avgLatencyMs);
}

void CDummy::NotifyLoopbackDataError(int id, __int64 recvData, __int64 expectedData)
{
	(void)id;
	(void)recvData;
	(void)expectedData;
	m_nLoopbackDataErrorCount.fetch_add(1);
}

void CDummy::NotifyChangePidError(int id, int errorCode)
{
	(void)id;
	(void)errorCode;
	m_nChangePidErrorCount.fetch_add(1);
}

CDummy g_DummyManager;
