#include "ScheduleDefines.h"
#include "../Log/CLog.h"
#include "../CUtill/CUtill.h"
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
	double time = CUtil::GetQPCNowTime();
	
	if (pClient->GetState() == ESTATE::IDEL)
	{
		return false;
	}
	else if (pClient->GetState() == ESTATE::MOVE_START)
	{
		pClient->SetState(ESTATE::MOVE_ING);
		StartTime = time;
		UpdateTime = time;
	}
	else if (pClient->GetState() == ESTATE::MOVE_STOP)
	{
		pClient->Arrive(EndPos);
		pClient->SetState(ESTATE::IDEL);
		return true;
	}
	double frame = (time - UpdateTime) / FIXED_DELTA;


	float speed = pClient->GetSpeed() * FIXED_DELTA * frame;

	float speedDx = Dir.X * speed;
	float speedDy = Dir.Y * speed;
	float speedDz = Dir.Z * speed;

	float speeddist = speed * speed;
	float remaindist = StartPos.DistanceToNSquared(EndPos);

	// Test 를 위해서 넘어가도 계속 더한다
	//if (remaindist <= speeddist)
	//	pClient->Arrive(EndPos);
	
	pClient->AddPos({ speedDx, speedDy, speedDz });

	UpdateTime = CUtil::GetQPCNowTime();
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
	st_CTS_Teleport req;
	req.pos = GoalPos;

	CPacket cPacket;
	cPacket << req;

	pClient->SendEnqueuePacket((*cPacket.GetBufferPtr()), &cPacket);

	return true;
}

bool st_Schedule_Teleport::DoSchedule(CClient* pClient)
{
	if (pClient->GetPos() == GoalPos)
		return true;
	return false;
}

bool st_Schedule_Delay::DoInitRunSchedule(CClient* pClient)
{
	return true;
}

bool st_Schedule_Delay::DoSchedule(CClient* pClient)
{
	// 일단 그냥 대기만 넣기
	return false;
}
