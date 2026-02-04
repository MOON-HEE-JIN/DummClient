#include "CClient.h"
#include "CPacketProc.h"
#include "CUtill/CUtill.h"

static CPacketProc g_cPacketProc;

CClient::CClient()
{
	m_LoopbackData = static_cast<__int64>(CUtil::Random(1000, 9999));
	m_nSendReqDelayTime = CUtil::Random(1, 5) * 1000;	// 1~5√ 
	m_nLastSendReqTime = GetTickCount();
}

void CClient::OnRecv(int type, CPacket& cPacket)
{
	printf("CClient::OnRecv Type : %d \n", type);
	g_cPacketProc.DO_GAME_Proc(type, this, cPacket);
}

void CClient::SendLoopbackPacket()
{
	st_CTS_LoopBack data;
	data.data = m_LoopbackData;
	CPacket pRes;
	pRes << data;
	
	SendEnqueuePacket(GAME::LOOPBACK, &pRes);
}

bool CClient::IsSend()
{
	if (m_nSendReqDelayTime + m_nLastSendReqTime < GetTickCount())
	{
		m_nLastSendReqTime = GetTickCount();
		return true;
	}
	return false;
}
