#include "CPacketProc.h"
#include "Stub/EnumDef.h"
#include "Log/CLog.h"
#include "CDummy/DummyDef.h"
#include "CDummy/CDummyManager.h"
#include "CUtill/CUtill.h"


int CPacketProc::DO_GAME_LOOPBACK(CClient* pTarget, CPacket& pReqPacket)
{
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

int CPacketProc::DO_GAME_CHANGEZONE(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_ChangeZone res;
	pReqPacket >> res;

	if (res.ret != 0)
	{
		g_LogDummy.ELog("ERROR Change Zone Result ret : %d", res.ret);
		return 0;
	}

	if(res.zone == 0)
	{
		return 0;
	}

	pTarget->SetChangeZone(res.channel, res.zone);

	return 0;
}

int CPacketProc::DO_GAME_CREATECHAR(CClient* pTarget, CPacket& pReqPacket)
{
	return 0;
}

int CPacketProc::DO_GAME_LEAVEZONE(CClient* pTarget, CPacket& pReqPacket)
{
	return 0;
}

int CPacketProc::DO_GAME_CONNECTINFO(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_ConnectInfo res;
	pReqPacket >> res;

	pTarget->ConnectServerLoginThread(res.info.ID);
	

	g_DummyManager.AddDummyClient(pTarget);

	return 0;
}

int CPacketProc::DO_GAME_ENTERZONE(CClient* pTarget, CPacket& pReqPacket)
{
	return 0;
}

int CPacketProc::DO_GAME_MOVESTART(CClient* pTarget, CPacket& pReqPacket)
{

	return 0;
}

int CPacketProc::DO_GAME_MOVESTOP(CClient* pTarget, CPacket& pReqPacket)
{
	return 0;
}

int CPacketProc::DO_OBSERVER_CONNET_OBSERVER(CClient* pTarget, CPacket& pReqPacket)
{
	return 0;
}

int CPacketProc::DO_GAME_CHANGEINGZONE(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_ChangeingZone res;
	pReqPacket >> res;

	int ret = res.ret;
	int type = res.type;
	return 0;
}

