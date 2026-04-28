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

	m_nSendReqDelayTime = 3 * 1000;	// 100~500 ms
}

void CClient::OnRecv(int type, CPacket& cPacket)
{
	//printf("CClient::OnRecv Type : %d \n", type);
	g_cPacketProc.DO_GAME_Proc(type, this, cPacket);
}

void CClient::SetChangeZoneVar()
{
	m_iStartChannel = m_iChannel;
	m_iStartZone = m_nZoneID;

	m_iEndChannel = m_iChannel;
	switch (m_iStartZone)
	{
	case 3:
		m_iEndZone = 1;
		break;
	case 6:
		m_iEndZone = 4;
		break;

	case 2:
		m_iEndZone = 1;
		break;
	case 5:
		m_iEndZone = 4;
		break;
	default:
		m_iStartZone = m_nZoneID;
		m_iEndZone = m_nZoneID;
	}
}

void CClient::CreateCharInfo(st_Vector3F pos, float speed)
{
	m_stPosition = pos;
	m_fSpeed = speed;
}

void CClient::SetServerClientID(int value)
{
	m_nServerClientID = value;
	g_DummyManager.RegisterServerIDtoClientID(value, m_nClientID);
}

void CClient::ReConnect(const char IP[16], unsigned short Port, HANDLE cicp)
{
	Connect(IP, Port, cicp);
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
	if (m_bLogin.load() == false)
		return false;

	if (m_bWaitServerResponse)
		return false;

	if (m_nSendReqDelayTime + m_nLastSendReqTime < GetTickCount())
	{
		m_nLastSendReqTime = GetTickCount();
		return true;
	}
	return false;
}

void CClient::MoveStop(st_Vector3F comparevector)
{
	double moveTime = CUtil::GetQPCNowTime() - m_dMoveStartTime;

	int nLoop = moveTime / FIXED_DELTA;

	m_stPosition += m_stDirection * (m_fSpeed * nLoop) * FIXED_DELTA;

	//st_Vector3F diff = m_stGoalPosition - m_stPosition;
	st_Vector3F compDiff = comparevector - m_stPosition;

	if (compDiff.Length() > 5)
		g_LogDummy.ELog("Move Complete Error : Compare Diff [%f]", compDiff.Length());
	else
		g_LogDummy.ELog("=== Move Complete : SUCESS [%d] ===", m_nClientID);

	m_stPosition = m_stGoalPosition;
}
