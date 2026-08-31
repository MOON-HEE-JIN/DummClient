#include "ScheduleDefines.h"
#include "../Log/CLog.h"
#include "../CUtill/CUtill.h"
#include "../Test/TSchedule_PlaceMainworld.h"

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
