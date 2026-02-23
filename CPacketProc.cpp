#include "CPacketProc.h"
#include "Stub/EnumDef.h"
#include "Log/CLog.h"
#include "CDummy/DummyDef.h"

int CPacketProc::DO_GAME_LOOPBACK(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_LoopBack data;
	pReqPacket >> data;

	int zone = data.zone;
	__int64 b = data.data;
	
	if (b != pTarget->GetLoopbackData())
	{
		g_LogDummy.ELog("LoopBack Data Error : %lld != %lld", b, pTarget->GetLoopbackData());
		return -1;
	}
	
	if (zone != pTarget->GetZoneID() && zone != 0)
	{
		g_LogDummy.ELog("Not Equal Zone Clinet[%d] - Server[%d]", pTarget->GetZoneID(), zone);
	}

#if __DUMMY_DISCONNECT__
	pTarget->IncrementDisConnectRandomCount();
#endif // __DUMMY_DISCONNECT__

#if __DUMMY_CHANGE_ZONE__
	pTarget->ChangeZoneIDRequest();
#endif //__DUMMY_CHANGE_ZONE__

	st_CTS_LoopBack res;
	res.data = pTarget->IncrementLoopbackData();
	res.zone = pTarget->GetZoneID();

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
		g_LogDummy.ELog("Error Change Pid ret : %d", res.ret);
		pTarget->ReStoreZoneID();
	}

	pTarget->CompletedWaitServerResponse();

	return 0;
}

