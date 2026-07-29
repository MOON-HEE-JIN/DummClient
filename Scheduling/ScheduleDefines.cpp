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
	switch (pClient->GetChannel())
	{
	case 0:
		iChannel = 1;
		break;
	case 1:
		iChannel = 0;
		break;
	default:
		iChannel = 2;
		break;
	}
	switch (pClient->GetZoneID())
	{
	case 3: // proc 1
	case 6: // proc 1
		iZoneID = 1;
		break;
	case 1: // proc 2
		iZoneID = 4;
		break;
	case 4: // proc 2
		iZoneID = 1;
		break;
	case 2: // proc 3
	case 5: // proc 3
		iZoneID = 4;
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
	// 로그인 스레드에서 벗어나기 위해 Zone 이동 패킷 전송
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

	return true;
}

bool st_Schedule_LoopBack::DoSchedule(CClient* pClient)
{
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
