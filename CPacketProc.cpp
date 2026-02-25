#include "CPacketProc.h"
#include "Stub/EnumDef.h"
#include "Log/CLog.h"
#include "CDummy/DummyDef.h"
#include "CDummy/CDummy.h"


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

	pTarget->SendEnqueuePacket(&pRes);
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

	res.ret;
	if (res.ret != ERROR_CODE::NOT_ERROR && res.ret != ERROR_CODE::EQUAL_PID)
	{
		g_LogDummy.ELog("Error Change Pid ret : %d %d", res.ret, res.zone);
		pTarget->ReStoreZoneID();
		pTarget->CompletedWaitServerResponse();
		return 0;
	}

	pTarget->CompletedWaitServerResponse();

	{
		st_CTS_EnterZone res;
		res.zone = pTarget->GetZoneID();

		CPacket pack;
		pack << res;
		pTarget->SendEnqueuePacket(&pack);
	}

	return 0;
}

int CPacketProc::DO_GAME_CREATECHAR(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_CreateChar res;
	pReqPacket >> res;

	if (g_DummyManager.GetServerIDtoClientID(res.ID) != pTarget->GetClientID())
	{
		g_LogDummy.ELog("NOT EQUAL ID [%d] != [%d]", g_DummyManager.GetServerIDtoClientID(res.ID), pTarget->GetClientID());
		return 0;
	}


	pTarget->CreateCharInfo(res.pos);
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
	
	pTarget->SetServerClientID(res.info.ID);
	g_LogDummy.ILog("Connect Server [%d] :: [%d]", res.info.ID, pTarget->GetClientID());
	return 0;
}

int CPacketProc::DO_GAME_ENTERZONE(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_EnterZone res;
	pReqPacket >> res;

	if (res.ret != 0)
	{
		g_LogDummy.ELog("ERROR DO_GAME_ENTERZONE");
		return 0;
	}
	
	int count = res.Loop1;
	int ret = 0;
	for (int i = 0; i < count; i++)
	{
		ret = g_DummyManager.IsExistZoneClient(pTarget->GetZoneID(), res.info[i].ID);
		if (ret == 0)
			continue;

		switch (ret)
		{
		case 1:
			g_LogDummy.ELog("ENTERZONE ERROR RET NOT_EXIST_CLIENT");
			break;
		case 3:
			g_LogDummy.ELog("ENTERZONE ERROR RET NOT_EQUAL_ZONE");
			break;
		default:
			g_LogDummy.ELog("ENTERZONE ERROR");
			break;
		}

		res.info[i].type;
		res.info[i].ID;
		res.info[i].pos;
	}
	return 0;
}

