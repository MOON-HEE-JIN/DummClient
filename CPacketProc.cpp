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
	pTarget->SpawnPos(res.spawn);
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
		pTarget->Arrive(res.pos);
		pTarget->SetState(ESTATE::MOVE_STOP);
		return 0;
	}

	// 서버 위치를 기준으로 보정한 뒤 로컬 이동 시뮬레이션을 시작한다.
	pTarget->Arrive(res.pos);
	pTarget->SetState(ESTATE::MOVE_START);
	return 0;
}

int CPacketProc::DO_GAME_MOVESTOP(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_MoveStop res;
	pReqPacket >> res;

	if (res.ret != 0)
	{
		g_LogDummy.ELog(
			"MOVE_STOP_ERROR ret[%d] server[%f,%f,%f] client[%f,%f,%f]",
			res.ret,
			res.pos.X,
			res.pos.Y,
			res.pos.Z,
			pTarget->GetPos().X,
			pTarget->GetPos().Y,
			pTarget->GetPos().Z);
		pTarget->Arrive(res.pos);
		pTarget->SetState(ESTATE::MOVE_STOP);
		return 0;
	}

	// 직접 요청의 응답은 현재 서버에서 ID가 채워지지 않을 수 있어 이동 상태도 함께 확인한다.
	if (res.ID == pTarget->GetServerID()
		|| pTarget->GetState() == ESTATE::MOVE_ING)
	{
		pTarget->Arrive(res.pos);
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
		if (pSchedule->GetType() != ESCHEDULE_TYPE::SCHEDULE_TYPE_TELEPORT
			&& pSchedule->GetType() != ESCHEDULE_TYPE::SCHEDULE_TYPE_MAIN_WORLD_TELEPORT)
			return 0;

		// 요청값 대신 서버 응답 위치를 사용해 이후 이동 오차가 누적되지 않게 한다.
		pTarget->Arrive(res.pos);
		pTarget->SetTeleportResult(0);
	}
	else
	{
		// 서버가 성공 응답 뒤 bool true를 오류 코드 1로 재전송하는 경우 중복 응답을 무시한다.
		if (pTarget->GetTeleportResult() == 0)
			return 0;

		pTarget->SetTeleportResult(res.ret);
		g_LogDummy.ELog("TELEPORT_ERROR Client[%d] ret : %d", pTarget->GetID(), res.ret);
	}
	return 0;
}

int CPacketProc::DO_GAME_AOI_IN_PLAYER(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_AoiInPlayer res;
	pReqPacket >> res;
	pTarget->AddVisiblePlayer(res.info.ID);
	return 0;
}

int CPacketProc::DO_GAME_AOI_IN_PLAYERS(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_AoiInPlayers res;
	pReqPacket >> res;

	if (res.Loop1 < 0 || res.Loop1 > 50)
	{
		g_LogDummy.ELog("AOI_IN_PLAYERS Invalid Count : %d", res.Loop1);
		return 0;
	}

	for (int i = 0; i < res.Loop1; ++i)
		pTarget->AddVisiblePlayer(res.info[i].ID);

	return 0;
}

int CPacketProc::DO_GAME_AOI_OUT_PLAYER(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_AoiOutPlayer res;
	pReqPacket >> res;
	pTarget->RemoveVisiblePlayer(res.ID);
	return 0;
}

int CPacketProc::DO_GAME_AOI_OUT_PLAYERS(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_AoiOutPlayers res;
	pReqPacket >> res;

	if (res.Loop1 < 0 || res.Loop1 > 50)
	{
		g_LogDummy.ELog("AOI_OUT_PLAYERS Invalid Count : %d", res.Loop1);
		return 0;
	}

	for (int i = 0; i < res.Loop1; ++i)
		pTarget->RemoveVisiblePlayer(res.info[i].ID);

	return 0;
}

int CPacketProc::DO_GAME_OTHERMOVESTART(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_OtherMoveStart res;
	pReqPacket >> res;
	pTarget->AddVisiblePlayer(res.ID);
	return 0;
}

int CPacketProc::DO_GAME_AOI_IN_PLAYERS_MOVE(CClient* pTarget, CPacket& pReqPacket)
{
	st_STC_AoiInPlayerMoves res;
	pReqPacket >> res;

	res.Loop1;
	res.move[0].ID;
	res.move[1].dir;
	res.move[1].pos;

	return 0;
}

