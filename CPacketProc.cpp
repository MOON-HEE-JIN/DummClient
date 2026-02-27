#include "CPacketProc.h"
#include "Stub/EnumDef.h"
#include "Log/CLog.h"
#include "CDummy/DummyDef.h"
#include "CDummy/CDummy.h"
#include "CUtill/CUtill.h"


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


	pTarget->CreateCharInfo(res.pos, res.speed);
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
		{
			// Zone 입장이 성공 했으면 테스트 시작
			pTarget->DummyTestPacketSend();
			continue;
		}

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

int CPacketProc::DO_GAME_MOVESTART(CClient* pTarget, CPacket& pReqPacket)
{
	return 0;
}

int CPacketProc::DO_GAME_MOVESTOP(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_MoveStop res;
	pReqPacket >> res;

	res.type;	// 유저 인지 몬스터인지 구분하기 위한 타입 이후 추가 예정
	res.ID;		// 구분값

	if (res.ret != 0)
	{
		g_LogDummy.ELog("MOVE COMPLETE ERROR RET : %d", res.ret);
		return 0;
	}

	switch (res.type)
	{
	case 0:
	{
		if (res.ID == pTarget->GetServerClientID())
		{
			pTarget->MoveStop(res.pos);
			break;
		}

		// SendBroadcastPacket 으로 다른 클라이언트의 MoveStop 패킷이 왔을 때, 해당 클라이언트의 위치를 확인 로직
		CClient* pOtherClient = g_DummyManager.GetClientByServerID(res.ID);

		if(pOtherClient == nullptr)
		{
			g_LogDummy.ELog("MOVE COMPLETE ERROR NOT EXIST CLIENT [%d]", res.ID);
			break;
		}
		if (pOtherClient->GetPosition() != res.pos)
		{
			// 아직 pOtherClient 의 위치가 업데이트 되지 않은 상태에서 MoveStop 패킷이 도착했을 때, MoveStop 패킷에 담긴 위치와 pOtherClient의 위치가 다를 수 있음
			if (pOtherClient->GetGoalPosition() != res.pos)
				g_LogDummy.ELog("MOVE COMPLETE ERROR NOT EQUAL POS [%f, %f, %f] != [%f, %f, %f]", pOtherClient->GetPosition().X, pOtherClient->GetPosition().Y, pOtherClient->GetPosition().Z, res.pos.X, res.pos.Y, res.pos.Z);
		}
		break;
	}
	default:
		g_LogDummy.ELog("MOVE COMPLETE ERROR NOT EQUAL ID [%d] != [%d]", res.ID, pTarget->GetServerClientID());
		break;
	}
	return 0;
}

