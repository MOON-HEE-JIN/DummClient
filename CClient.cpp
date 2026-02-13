#include "CClient.h"
#include "CPacketProc.h"
#include "CUtill/CUtill.h"
#include "CDummy/CDummy.h"

static CPacketProc g_cPacketProc;

CClient::CClient(int ClientID)
{
	m_nClientID = ClientID;

	m_LoopbackData = static_cast<__int64>(CUtil::Random(1000, 9999));
	m_nLastSendReqTime = GetTickCount();
	m_nLastLoopbackSendTick = 0;
	m_nLastLoopbackLatencyMs = 0;
	m_nAvgLoopbackLatencyMs = 0;
	m_nLoopbackSendCount = 0;

	m_nSendReqDelayTime = CUtil::Random(1, 5) * 1000;	// 1~5 초

	m_nDisConnectRandomCount = CUtil::Random(10, 40); // 10~40

	m_nServerProcID = CUtil::Random(1, 6);
	m_nDisConnectRandomCount = CUtil::Random(5, 10); // 2000~5000
	m_nForceDisconnectTick = GetTickCount() + (CUtil::Random(5, 20) * 1000);
	m_bWaitChangePidAck = false;
}

void CClient::OnRecv(int type, CPacket& cPacket)
{
	g_cPacketProc.DO_GAME_Proc(type, this, cPacket);
}

void CClient::SendChangePidPacket()
{
	if (m_bWaitChangePidAck)
		return;

	st_CTS_ChangePid data;
	data.pid = m_nServerProcID;
	
	CPacket Req;
	Req << data;

	SendEnqueuePacket(GAME::CHANGEPID, &Req);
	m_bWaitChangePidAck = true;
}

void CClient::SendLoopbackPacket()
{
	if (m_bWaitChangePidAck)
		return;

	st_CTS_LoopBack data;
	data.data = m_LoopbackData;
	CPacket pRes;
	pRes << data;
	
	SendEnqueuePacket(GAME::LOOPBACK, &pRes);
	m_nLastLoopbackSendTick = GetTickCount();
}

bool CClient::IncrementDisConnectRandomCount()
{
	if (++m_nLoopbackSendCount > m_nDisConnectRandomCount)
	{
		DisConnect();
		g_DummyManager.DisconnectClient(m_nClientID);
		g_DummyManager.NotifyClientDisconnected(m_nClientID, false);
		return true;
	}
	return false;
}

void CClient::OnChangePidAck()
{
	m_bWaitChangePidAck = false;
}

void CClient::OnLoopbackAck()
{
	if (m_nLastLoopbackSendTick <= 0)
		return;

	m_nLastLoopbackLatencyMs = GetTickCount() - m_nLastLoopbackSendTick;
	if (m_nAvgLoopbackLatencyMs == 0)
		m_nAvgLoopbackLatencyMs = m_nLastLoopbackLatencyMs;
	else
		m_nAvgLoopbackLatencyMs = (m_nAvgLoopbackLatencyMs * 9 + m_nLastLoopbackLatencyMs) / 10;
}

bool CClient::IsSend()
{
	if (m_nForceDisconnectTick < GetTickCount())
	{
		DisConnect();
		g_DummyManager.DisconnectClient(m_nClientID);
		g_DummyManager.NotifyClientDisconnected(m_nClientID, true);
		return false;
	}

	if (m_bWaitChangePidAck)
		return false;

	if (m_nSendReqDelayTime + m_nLastSendReqTime < GetTickCount())
	{
		m_nLastSendReqTime = GetTickCount();
		return true;
	}
	return false;
}
