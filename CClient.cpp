#include "CClient.h"
#include "CPacketProc.h"
#include "CUtill/CUtill.h"
#include "CDummy/CDummy.h"

static CPacketProc g_cPacketProc;

CClient::CClient(int ClientID, int testZone, int inZoneClientID)
{
	m_nClientID = ClientID;
	m_nTestZoneID = testZone;
	m_nInZoneClientID = inZoneClientID;

	m_LoopbackData = static_cast<__int64>(CUtil::Random(1000, 9999));
	m_nLastSendReqTime = GetTickCount();
	m_nLoopbackSendCount = 0;

	m_nSendReqDelayTime = CUtil::Random(1, 5) * 1000;	// 1~5 초

	m_nDisConnectRandomCount = CUtil::Random(2000, 5000); // 2000~5000
}

void CClient::OnRecv(int type, CPacket& cPacket)
{
	//printf("CClient::OnRecv Type : %d \n", type);
	g_cPacketProc.DO_GAME_Proc(type, this, cPacket);
}

void CClient::SendChangePidPacket()
{
	st_CTS_ChangePid data;
	data.pid = m_nTestZoneID;
	
	CPacket Req;
	Req << data;

	SendPacket(GAME::CHANGEPID, &Req);
	m_bWaitServerResponse = true; // 응답 대기 전환
}

void CClient::SendLoopbackPacket()
{
	st_CTS_LoopBack data;
	data.data = m_LoopbackData;
	CPacket pRes;
	pRes << data;
	
	SendEnqueuePacket(GAME::LOOPBACK, &pRes);
}

bool CClient::IncrementDisConnectRandomCount()
{
	if (++m_nLoopbackSendCount > m_nDisConnectRandomCount)
	{
		DisConnect();
		g_DummyManager.DisconnectClient(m_nClientID);
		return true;
	}
	return false;
}

bool CClient::IsSend()
{
	if (m_bWaitServerResponse)
		return false;

	if (m_nSendReqDelayTime + m_nLastSendReqTime < GetTickCount())
	{
		m_nLastSendReqTime = GetTickCount();
		return true;
	}
	return false;
}
