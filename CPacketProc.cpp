#include "CPacketProc.h"


int CPacketProc::DO_GAME_LOOPBACK(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_LoopBack data;
	pReqPacket >> data;
	
	__int64 b = data.data;
	
	if (b != pTarget->GetLoopbackData())
	{
		printf("LoopBack Data Error : %lld != %lld\n", b, pTarget->GetLoopbackData());
		return -1;
	}
	
	st_CTS_LoopBack res;
	res.data = pTarget->IncrementLoopbackData();
	
	CPacket pRes;
	pRes << res;

	pTarget->SendEnqueuePacket(GAME::LOOPBACK, &pRes);
	return 0;
}

int CPacketProc::DO_ERROR_PACKET(CClient* pTarget, CPacket& pReqPacket)
{
	return 0;
}
int CPacketProc::DO_ERROR_RESULT(CClient* pTarget, int ret, int type)
{
	return 0;
}

