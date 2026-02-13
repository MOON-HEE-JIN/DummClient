#include "CPacketProc.h"
#include "Stub/EnumDef.h"
#include "CDummy/CDummy.h"

int CPacketProc::DO_GAME_LOOPBACK(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_LoopBack data;
	pReqPacket >> data;
	
	__int64 b = data.data;
	
	if (b != pTarget->GetLoopbackData())
	{
		g_DummyManager.NotifyLoopbackDataError(pTarget->GetClientID(), b, pTarget->GetLoopbackData());
		return -1;
	}

	pTarget->OnLoopbackAck();
	g_DummyManager.NotifyLoopbackLatency(pTarget->GetClientID(), pTarget->GetLastLoopbackLatencyMs(), pTarget->GetAvgLoopbackLatencyMs());
	
	pTarget->IncrementDisConnectRandomCount();

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

int CPacketProc::DO_GAME_CHANGEPID(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_ChangePid res;
	pReqPacket >> res;

	res.ret;
	if (res.ret != ERROR_CODE::NOT_ERROR && res.ret != ERROR_CODE::EQUAL_PID)
	{
		g_DummyManager.NotifyChangePidError(pTarget->GetClientID(), res.ret);
	}

	pTarget->OnChangePidAck();
	pTarget->SendLoopbackPacket();
	return 0;
}
