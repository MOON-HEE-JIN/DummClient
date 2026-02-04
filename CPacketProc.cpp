#include "CPacketProc.h"


int CPacketProc::DO_GAME_LOOPBACK(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_LoopBack data;
	pReqPacket >> data;
	
	__int64 b = data.data;
	
	b++;

	CPacket pRes;
	st_CTS_LoopBack res;
	res.data = b;
	pRes << res;
	Sleep(1000 * 5);
	pTarget->SendPacket(GAME::LOOPBACK, &pRes);
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

