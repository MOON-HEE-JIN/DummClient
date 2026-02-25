#include "CClient.h"
#include "CPacketProc.h"
#include "CUtill/CUtill.h"
#include "CDummy/CDummy.h"
#include "CDummy/DummyDef.h"
#include "Log/CLog.h"
static CPacketProc g_cPacketProc;

CClient::CClient(int ClientID, int testZone, int inZoneClientID)
{
	m_nClientID = ClientID;
	m_nZoneID = testZone;
	m_nInZoneClientID = inZoneClientID;

	m_LoopbackData = static_cast<__int64>(CUtil::Random(1000, 9999));
	m_nLastSendReqTime = GetTickCount();

	m_nLoopbackSendCount = 0;
	m_nDisConnectRandomCount = CUtil::Random(2000, 5000); // 2000~5000

	m_nChangeZoneIDCount = 0;
	m_nChangeZoneIDRequestCount = CUtil::Random(20, 800);

	m_nSendReqDelayTime = CUtil::Random(1, 5) * 100;	// 100~500 ms
}

void CClient::OnRecv(int type, CPacket& cPacket)
{
	//printf("CClient::OnRecv Type : %d \n", type);
	g_cPacketProc.DO_GAME_Proc(type, this, cPacket);
}

void CClient::SendChangePidPacket()
{
	if (m_bWaitServerResponse.load())
		return;
	m_bWaitServerResponse.store(true); // 응답 대기 전환
	st_CTS_ChangeZone data;
	data.zone = m_nZoneID;
	
	CPacket Req;
	Req << data;

	int len = SendPacket(&Req);
	if (len > sizeof(st_CTS_ChangeZone) + sizeof(st_Header))
	{
		int a = 100;
		a++;
	}
}

void CClient::SendLoopbackPacket()
{
	st_CTS_LoopBack data;
	data.data = m_LoopbackData;
	data.zone = GetZoneID();
	CPacket pRes;
	pRes << data;
	
	SendEnqueuePacket(&pRes);
}

void CClient::ReConnect(const char IP[16], unsigned short Port, HANDLE cicp)
{
	Connect(IP, Port, cicp);
}

void CClient::CreateCharInfo(st_Vector pos)
{
	m_stPosition = pos;
}

void CClient::SetServerClientID(int value)
{
	m_nServerClientID = value;
	g_DummyManager.RegisterServerIDtoClientID(value, m_nClientID);
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

void CClient::ChangeZoneIDRequest()
{
	static std::atomic<bool> onLog = false;
	bool t = false;
	if (++m_nChangeZoneIDCount > m_nChangeZoneIDRequestCount)
	{
		m_nPreZoneID = m_nZoneID;
		m_nZoneID = CUtil::Random(1, MAX_ZONE_NUMBER);
		m_nChangeZoneIDCount = 0;
		SendChangePidPacket();
		if (onLog.compare_exchange_strong(t, true))
		{
			g_LogDummy.ILog("====Start Change Zone ID====");
		}
	}
}

bool CClient::IsSend()
{
	if (m_bWaitServerResponse)
		return false;

	if (m_bWaitServerResponse)
	{
		int a = 100;
		a++;
	}
	if (m_nSendReqDelayTime + m_nLastSendReqTime < GetTickCount())
	{
		m_nLastSendReqTime = GetTickCount();
		return true;
	}
	return false;
}
