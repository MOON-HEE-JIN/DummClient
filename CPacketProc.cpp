#include "CPacketProc.h"
#include "Stub/EnumDef.h"
#include "Log/CLog.h"
#include "CDummy/DummyDef.h"
#include "CDummy/CDummyManager.h"
#include "CUtill/CUtill.h"


int CPacketProc::DO_GAME_LOOPBACK(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_LoopBack res;
	pReqPacket >> res;

	if (res.ret != 0)
		pTarget->SetRecvLoopBack(-1);
	else
		pTarget->SetRecvLoopBack(res.data);

	res.ret;
	res.data;
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

int CPacketProc::DO_GAME_CONNECTINFO(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_ConnectInfo res;
	pReqPacket >> res;

	pTarget->ConnectServerLoginThread(res.info.ID);
	

	g_DummyManager.AddDummyClient(pTarget);

	return 0;
}

int CPacketProc::DO_GAME_MOVESTART(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_MoveStart res;
	pReqPacket >> res;

	if (res.ret != 0)
	{
		g_LogDummy.ELog("MOVE_START_ERROR rst : %d  sPos[%f,%f,%f], cPos[%f,%f,%f]", res.ret, res.pos.X, res.pos.Y, res.pos.Z, pTarget->GetPos().X, pTarget->GetPos().Y, pTarget->GetPos().Z);
	}

	pTarget->SetState(ESTATE::MOVE_START);
	return 0;
}

int CPacketProc::DO_GAME_MOVESTOP(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_MoveStop res;
	pReqPacket >> res;

	if (res.ret != 0)
	{
		g_LogDummy.ELog("MOVE_STOP_ERROR");
		return 0;
	}

	if (res.ID == pTarget->GetServerID())
	{
		int diff = pTarget->GetPos().DistanceToSquared(res.pos);

		//if (diff >= 10)
		//	g_LogDummy.ELog("CheckFrames diff : %d", diff);

		pTarget->SetState(ESTATE::MOVE_STOP);
	}

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

int CPacketProc::DO_GAME_TELEPORT(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_Teleport res;
	pReqPacket >> res;

	if (res.ret == 0)
	{
		const st_Schedule* pSchedule = pTarget->GetWorkSchedule();
		if (pSchedule == nullptr)
			return 0;
		if (pSchedule->GetType() != ESCHEDULE_TYPE::SCHEDULE_TYPE_TELEPORT)
			return 0;

		pTarget->Arrive(((st_Schedule_Teleport*)pSchedule)->GoalPos);
	}
	return 0;
}

int CPacketProc::DO_GAME_AOI_IN_PLAYER(CClient* pTarget, CPacket& pReqPacket)
{
	return 0;
}

int CPacketProc::DO_GAME_AOI_IN_PLAYERS(CClient* pTarget, CPacket& pReqPacket)
{
	return 0;
}

int CPacketProc::DO_GAME_AOI_OUT_PLAYER(CClient* pTarget, CPacket& pReqPacket)
{
	return 0;
}

int CPacketProc::DO_GAME_AOI_OUT_PLAYERS(CClient* pTarget, CPacket& pReqPacket)
{
	return 0;
}

int CPacketProc::DO_GAME_OTHERMOVESTART(CClient* pTarget, CPacket& pReqPacket)
{
	return 0;
}

