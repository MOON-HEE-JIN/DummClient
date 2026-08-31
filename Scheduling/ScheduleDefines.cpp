#include "ScheduleDefines.h"
#include "../Log/CLog.h"
#include "../CUtill/CUtill.h"
#include "../Test/TSchedule_PlaceMainworld.h"
#include "../Test/TSchedule_MainWorldMoveAoi.h"

namespace
{
	void SendMoveStop(CClient* pClient, const st_Vector3F& pos)
	{
		st_CTS_MoveStop req;
		req.pos = pos;

		CPacket packet;
		packet << req;
		pClient->SendEnqueuePacket(*packet.GetBufferPtr(), &packet);
	}
}
bool st_Schedule_Login::DoSchedule(CClient* pClient)
{
	if (pClient->GetLogin())
	{
		bComplete = true;
		return true;
	}
	return false;
}

bool st_Schedule_LoginChangeZone::DoInitRunSchedule(CClient* pClient)
{
	CPacket packet;
	st_CTS_ChangeZone req;
	// 로그인 스레드에서 벗어나기 위해 Zone 이동 패킷 전송
	req.zone = pClient->GetDeafultZoneID();
	req.channel = pClient->GetDefaultChannel();

	packet << req;

	pClient->SendEnqueuePacket(*packet.GetBufferPtr(), &packet);

	return true;
}

bool st_Schedule_LoginChangeZone::DoSchedule(CClient* pClient)
{
	if (pClient->GetZoneID() == pClient->GetDeafultZoneID()
		&& pClient->GetChannel() == pClient->GetDefaultChannel())
	{
		bComplete = true;
		return true;
	}
	return false;
}

bool st_Schedule_ChangeZone::DoInitRunSchedule(CClient* pClient)
{
	// channel 0, 1
	// zone 2,3,4,5,6  -  2,3,4,5,6
	/*
		
		Proc[2] channel : 0 Zone[2]		  Proc[3] channel : 1 Zone[3]
		Proc[2] channel : 0 Zone[4]		  Proc[3] channel : 1 Zone[5]
		Proc[2] channel : 0 Zone[6]		  Proc[3] channel : 1 Zone[7]
		
		Proc[2] channel : 1 Zone[2]		  Proc[3] channel : 0 Zone[3]
		Proc[2] channel : 1 Zone[4]		  Proc[3] channel : 0 Zone[5]
		Proc[2] channel : 1 Zone[6]		  Proc[3] channel : 0 Zone[7]
	*/
	switch (pClient->GetChannel())
	{
	case 0:
		iChannel = 1;
		break;
	case 1:
		iChannel = 0;
		break;
	default:
		break;
	}
	switch (pClient->GetZoneID())
	{
	case 2:
		iZoneID = 3;
		break;
	case 3:
		iZoneID = 2;
		break;
	case 4:
		iZoneID = 5;
		break;
	case 5:
		iZoneID = 4;
		break;
	case 6:
		iZoneID = 7;
		break;
	case 7:
		iZoneID = 6;
		break;
	default:
		iZoneID = 6;
		break;
	}

	CPacket packet;
	st_CTS_ChangeZone req;

	req.channel = iChannel;
	req.zone = iZoneID;

	packet << req;

	pClient->SendEnqueuePacket(*packet.GetBufferPtr(), &packet);

	return true;
}

bool st_Schedule_ChangeZone::DoSchedule(CClient* pClient)
{
	if (pClient->GetZoneID() == iZoneID
		&& pClient->GetChannel() == iChannel)
	{
		bComplete = true;
		return true;
	}
	return false;
}

bool st_Schedule_ReturnZone::DoInitRunSchedule(CClient* pClient)
{
	iZoneID = pClient->GetDeafultZoneID();
	iChannel = pClient->GetDefaultChannel();

	CPacket packet;
	st_CTS_ChangeZone req;

	req.zone = iZoneID;
	req.channel = iChannel;

	packet << req;

	pClient->SendEnqueuePacket(*packet.GetBufferPtr(), &packet);

	return true;
}

bool st_Schedule_ReturnZone::DoSchedule(CClient* pClient)
{
	if (pClient->GetZoneID() == iZoneID
		&& pClient->GetChannel() == iChannel)
	{
		bComplete = true;
		return true;
	}
	return false;
}

bool st_Schedule_MoveStart::DoInitRunSchedule(CClient* pClient)
{
	StartPos = pClient->GetPos();
	EndPos = CUtil::RandomVector3F(-80, 80);
	Dir = StartPos.Direction(EndPos);
	bStopRequested = false;
	pClient->SetState(ESTATE::IDEL);

	st_CTS_MoveStart req;
	req.pos = StartPos;
	req.goal = EndPos;
	req.dir = Dir;

	CPacket packet;
	packet << req;

	pClient->SendEnqueuePacket((*packet.GetBufferPtr()), &packet);
	
	return true;
}

bool st_Schedule_MoveStart::DoSchedule(CClient* pClient)
{
	const double time = CUtil::GetQPCNowTime();
	
	if (pClient->GetState() == ESTATE::IDEL)
	{
		return false;
	}
	else if (pClient->GetState() == ESTATE::MOVE_START)
	{
		pClient->SetState(ESTATE::MOVE_ING);
		StartTime = time;
		UpdateTime = time;
		StartPos = pClient->GetPos();
		Dir = StartPos.Direction(EndPos);
	}
	else if (pClient->GetState() == ESTATE::MOVE_STOP)
	{
		pClient->SetState(ESTATE::IDEL);
		return true;
	}

	if (bStopRequested)
		return false;

	const float moveDistance = pClient->GetSpeed() * static_cast<float>(time - UpdateTime);
	const float remainDistance = pClient->GetPos().DistanceToSquared(EndPos);
	if (remainDistance <= moveDistance)
	{
		pClient->Arrive(EndPos);
		SendMoveStop(pClient, EndPos);
		bStopRequested = true;
	}
	else
	{
		pClient->AddPos(Dir * moveDistance);
	}

	UpdateTime = time;
	return false;
}

bool st_Schedule_MainWorldTeleport::DoInitRunSchedule(CClient* pClient)
{
	StartPos = pClient->GetPos();
	return pOwner != nullptr;
}

bool st_Schedule_MainWorldTeleport::DoSchedule(CClient* pClient)
{
	if (pOwner == nullptr)
		return false;

	if (!bTeleportRequested)
	{
		if (!pOwner->CanStartCycle(Cycle))
			return false;

		StartPos = pClient->GetPos();
		GoalPos = pOwner->GetCyclePos(pClient->GetID(), Cycle);
		pClient->BeginTeleport();

		st_CTS_Teleport req;
		req.pos = GoalPos;
		CPacket packet;
		packet << req;
		pClient->SendEnqueuePacket(*packet.GetBufferPtr(), &packet);
		bTeleportRequested = true;
		return false;
	}

	const int result = pClient->GetTeleportResult();
	if (result == -1)
		return false;

	const st_Vector3F actual = pClient->GetPos();
	const bool teleportPass = result == 0
		&& actual.DistanceToSquared(GoalPos) < 0.1f;
	g_LogDummy.ILog(
		"MainWorld TELEPORT_%s Cycle[%d] Client[%d] Tile[%d,%d]->[%d,%d] Grid[%d]->[%d] Visible[%d]",
		teleportPass ? "PASS" : "FAIL",
		Cycle,
		pClient->GetID(),
		pOwner->GetTileX(StartPos),
		pOwner->GetTileZ(StartPos),
		pOwner->GetTileX(actual),
		pOwner->GetTileZ(actual),
		pOwner->GetManagementGrid(StartPos),
		pOwner->GetManagementGrid(actual),
		pClient->GetVisiblePlayerCount());

	int passCount = 0;
	if (pOwner->RecordCycleResult(Cycle, teleportPass, passCount))
	{
		g_LogDummy.ILog(
			"MainWorld TELEPORT SUMMARY Cycle[%d] Client[%d] Pass[%d] Fail[%d]",
			Cycle,
			pOwner->GetExpectedClientCount(),
			passCount,
			pOwner->GetExpectedClientCount() - passCount);
	}

	++Cycle;
	bTeleportRequested = false;
	return false;
}

bool st_Schedule_MainWorldMoveAoi::DoInitRunSchedule(CClient* pClient)
{
	if (pOwner == nullptr)
		return false;

	GoalPos = pOwner->GetStartPos(pClient->GetID());
	if (!pOwner->IsValidPosition(GoalPos))
		return false;

	pClient->BeginTeleport();
	st_CTS_Teleport req;
	req.pos = GoalPos;
	CPacket packet;
	packet << req;
	pClient->SendEnqueuePacket(*packet.GetBufferPtr(), &packet);

	Phase = EPhase::WAIT_TELEPORT;
	PhaseStartTime = CUtil::GetQPCNowTime();
	return true;
}

bool st_Schedule_MainWorldMoveAoi::DoSchedule(CClient* pClient)
{
	if (pOwner == nullptr)
		return false;

	const double now = CUtil::GetQPCNowTime();
	switch (Phase)
	{
	case EPhase::WAIT_TELEPORT:
	{
		const int teleportResult = pClient->GetTeleportResult();
		if (teleportResult == -1 && now - PhaseStartTime < 15.0)
			return false;

		if (teleportResult != 0
			|| pClient->GetPos().DistanceToNSquared(GoalPos) > 0.25f)
		{
			g_LogDummy.ELog(
				"MainWorld MOVE/AOI PLACE_RETRY Client[%d] ret[%d] Pos[%.1f,%.1f] Goal[%.1f,%.1f]",
				pClient->GetID(),
				teleportResult,
				pClient->GetPos().X,
				pClient->GetPos().Z,
				GoalPos.X,
				GoalPos.Z);
			DoInitRunSchedule(pClient);
			return false;
		}

		// 최초 AOI 생성 패킷이 모두 처리된 후 Move 검증 카운터를 초기화한다.
		pOwner->RecordPlacementReady(pClient->GetChannel());
		Phase = EPhase::PLACEMENT_SETTLE;
		PhaseStartTime = now;
		return false;
	}
	case EPhase::PLACEMENT_SETTLE:
		if (pOwner->CanStartCycle(pClient->GetChannel(), Cycle))
			StartMove(pClient);
		return false;
	case EPhase::WAIT_MOVE_START:
	{
		const int moveStartResult = pClient->GetMoveStartResult();
		if (moveStartResult == -1 && now - PhaseStartTime < 10.0)
			return false;

		if (moveStartResult != 0)
		{
			CompleteCycle(pClient, false, false);
			return false;
		}

		pClient->SetState(ESTATE::MOVE_ING);
		Phase = EPhase::WAIT_MOVE_STOP;
		PhaseStartTime = now;
		return false;
	}
	case EPhase::WAIT_MOVE_STOP:
	{
		const int moveStopResult = pClient->GetMoveStopResult();
		if (moveStopResult == -1)
		{
			if (now - PhaseStartTime >= 30.0)
			{
				// 서버 자동 도착 응답이 지연되면 명시적 Stop으로 이동 상태를 회수한다.
				SendMoveStop(pClient, pClient->GetPos());
				PhaseStartTime = now;
				g_LogDummy.ELog(
					"MainWorld MOVE/AOI MOVE_STOP_TIMEOUT Client[%d] Cycle[%d]",
					pClient->GetID(),
					Cycle);
			}
			return false;
		}

		if (moveStopResult != 0)
		{
			CompleteCycle(pClient, false, false);
			return false;
		}

		Phase = EPhase::AOI_SETTLE;
		PhaseStartTime = now;
		return false;
	}
	case EPhase::AOI_SETTLE:
	{
		if (now - PhaseStartTime < pOwner->GetAoiSettleTime())
			return false;

		const bool movePass = pClient->GetMoveStartResult() == 0
			&& pClient->GetMoveStopResult() == 0
			&& pClient->GetPos().DistanceToNSquared(GoalPos) <= 0.25f;
		// 월드 가장자리 방향에는 AOI IN 또는 OUT strip 자체가 없으므로 예상 이벤트만 검사한다.
		const bool expectAoiIn = pOwner->IsAoiInExpected(StartPos, GoalPos);
		const bool expectAoiOut = pOwner->IsAoiOutExpected(StartPos, GoalPos);
		const bool aoiPass = (!expectAoiIn || pClient->GetAoiInEntityCount() > 0)
			&& (!expectAoiOut || pClient->GetAoiOutEntityCount() > 0);
		CompleteCycle(pClient, movePass, aoiPass);
		return false;
	}
	case EPhase::MOVE_INTERVAL:
		if (pOwner->CanStartCycle(pClient->GetChannel(), Cycle))
			StartMove(pClient);
		return false;
	default:
		return false;
	}
}

void st_Schedule_MainWorldMoveAoi::StartMove(CClient* pClient)
{
	StartPos = pClient->GetPos();
	GoalPos = pOwner->GetMoveGoal(StartPos);
	if (!pOwner->IsValidPosition(GoalPos))
	{
		g_LogDummy.ELog(
			"MainWorld MOVE/AOI INVALID_GOAL Client[%d] Pos[%.1f,%.1f] Goal[%.1f,%.1f]",
			pClient->GetID(),
			StartPos.X,
			StartPos.Z,
			GoalPos.X,
			GoalPos.Z);
		CompleteCycle(pClient, false, false);
		return;
	}

	pClient->ResetAoiTransitionCount();
	pClient->BeginMove();
	pClient->SetState(ESTATE::IDEL);

	st_CTS_MoveStart req;
	req.pos = StartPos;
	req.goal = GoalPos;
	req.dir = StartPos.Direction(GoalPos);
	CPacket packet;
	packet << req;
	pClient->SendEnqueuePacket(*packet.GetBufferPtr(), &packet);

	Phase = EPhase::WAIT_MOVE_START;
	PhaseStartTime = CUtil::GetQPCNowTime();
}

void st_Schedule_MainWorldMoveAoi::CompleteCycle(
	CClient* pClient,
	bool movePass,
	bool aoiPass)
{
	const st_Vector3F actual = pClient->GetPos();
	const bool expectAoiIn = pOwner->IsAoiInExpected(StartPos, GoalPos);
	const bool expectAoiOut = pOwner->IsAoiOutExpected(StartPos, GoalPos);
	g_LogDummy.ILog(
		"MainWorld MOVE/AOI_%s Channel[%d] Cycle[%d] Client[%d]\n"
		"\tTile[%d,%d]->[%d,%d] Grid[%d]->[%d] Result[Move:%s AOI:%s] MoveRet[%d,%d]\n"
		"\tAOI[Expected:%d/%d In:%d/%d Out:%d/%d OtherMove:%d MoveSnapshot:%d Visible:%d]",
		(movePass && aoiPass) ? "PASS" : "FAIL",
		pClient->GetChannel(),
		Cycle,
		pClient->GetID(),
		pOwner->GetTileX(StartPos),
		pOwner->GetTileZ(StartPos),
		pOwner->GetTileX(actual),
		pOwner->GetTileZ(actual),
		pOwner->GetManagementGrid(StartPos),
		pOwner->GetManagementGrid(actual),
		movePass ? "PASS" : "FAIL",
		aoiPass ? "PASS" : "FAIL",
		pClient->GetMoveStartResult(),
		pClient->GetMoveStopResult(),
		expectAoiIn ? 1 : 0,
		expectAoiOut ? 1 : 0,
		pClient->GetAoiInTransitionCount(),
		pClient->GetAoiInEntityCount(),
		pClient->GetAoiOutTransitionCount(),
		pClient->GetAoiOutEntityCount(),
		pClient->GetOtherMoveStartCount(),
		pClient->GetAoiMoveEntityCount(),
		pClient->GetVisiblePlayerCount());

	TSchedule_MainWorldMoveAoi::st_CycleSummary summary;
	if (pOwner->RecordCycleResult(
		pClient->GetChannel(), Cycle, movePass, aoiPass, summary))
	{
		g_LogDummy.ILog(
			"\n=======================================================================\n"
			"\tMainWorld MOVE/AOI SUMMARY Channel[%d] Cycle[%d] Client[%d]\n"
			"\tMovePass[%d] MoveFail[%d] AoiPass[%d] AoiFail[%d]\n"
			"=======================================================================",
			pClient->GetChannel(),
			Cycle,
			summary.ClientCount,
			summary.MovePassCount,
			summary.ClientCount - summary.MovePassCount,
			summary.AoiPassCount,
			summary.ClientCount - summary.AoiPassCount);
	}

	++Cycle;
	Phase = EPhase::MOVE_INTERVAL;
	PhaseStartTime = CUtil::GetQPCNowTime();
}

bool st_Schedule_MoveStop::DoInitRunSchedule(CClient* pClient)
{
	StopPos = pClient->GetPos();

	st_CTS_MoveStop req;
	req.pos = StopPos;

	CPacket packet;
	packet << req;

	pClient->SendEnqueuePacket((*packet.GetBufferPtr()), &packet);
	return true;
}

bool st_Schedule_MoveStop::DoSchedule(CClient* pClient)
{
	return false;
}

bool st_Schedule_LoopBack::DoInitRunSchedule(CClient* pClient)
{
	data = CUtil::Random(0, 100000);
	st_CTS_LoopBack req;
	req.data = static_cast<double>(data);

	CPacket cPacket;
	cPacket << req;
	pClient->SendEnqueuePacket((*cPacket.GetBufferPtr()), &cPacket);
	SendEnqueueTime = CUtil::GetQPCNowTime();
	return true;
}

bool st_Schedule_LoopBack::DoSchedule(CClient* pClient)
{
	double time = CUtil::GetQPCNowTime();
	// 5s 이상 이라면
	if (time - SendEnqueueTime > 20)
	{
		g_LogDummy.ELog("LoopBack Schedule Over 20 Second");
		return false;
	}
	if (pClient->GetRecvLoopBack() == data)
		return true;
	return false;
}

bool st_Schedule_Teleport::DoInitRunSchedule(CClient* pClient)
{
	pClient->BeginTeleport();

	st_CTS_Teleport req;
	req.pos = GoalPos;

	CPacket cPacket;
	cPacket << req;

	pClient->SendEnqueuePacket((*cPacket.GetBufferPtr()), &cPacket);

	return true;
}

bool st_Schedule_Teleport::DoSchedule(CClient* pClient)
{
	if (pClient->GetTeleportResult() == 0 && pClient->GetPos() == GoalPos)
		return true;
	return false;
}

bool st_Schedule_Delay::DoInitRunSchedule(CClient* pClient)
{
	return true;
}

bool st_Schedule_Delay::DoSchedule(CClient* pClient)
{
	if (bInit == false)
	{
		StartTime = CUtil::GetQPCNowTime();
		bInit = true;
	}
	else
	{
		if (DelayTime != -1)
		{
			double time = CUtil::GetQPCNowTime();
			if (time - StartTime >= DelayTime)
				return true;
		}
	}
	return false;
}

bool st_Schedule_DisConnect::DoInitRunSchedule(CClient* pClient)
{
	// DisConnect 하면 Session 에 대한 delete 처리하면서 오류 발생 임시로 Cnt 증가하여 delete 방지
	pClient->IncrementIOCnt();
	pClient->DisConnect();
	DelayTime = CUtil::GetQPCNowTime();
	return true;
}

bool st_Schedule_DisConnect::DoSchedule(CClient* pClient)
{
	double time = CUtil::GetQPCNowTime();
	// DisConnect() 후 Server 에서 5초후에 Session 에 대한 정리를 한다
	// 로그로 끊김과 연결을 편히 확인하기 위해서는 10초후에 연결 해야한다
	if (time - DelayTime < 10)
		return false;
	if (pClient->GetLogin() == false)
		return true;
	return false;
}

bool st_Schedule_ReConnect::DoInitRunSchedule(CClient* pClient)
{
	pClient->ReConnect();
	return true;
}

bool st_Schedule_ReConnect::DoSchedule(CClient* pClient)
{
	if (pClient->GetConnect() == true)
	{
		// Disconnect 에서한 증가 했기에 연결시 감소
		pClient->DecrementIOCnt();
		return true;
	}
	return false;
}
