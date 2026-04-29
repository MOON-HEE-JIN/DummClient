#include "ScheduleDefines.h"
#include "../Log/CLog.h"
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

	pClient->SendEnqueuePacket(&packet);

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
}

bool st_Schedule_ChangeZone::DoInitRunSchedule(CClient* pClient)
{
	CPacket packet;
	st_CTS_ChangeZone req;
	switch (pClient->GetChannel())
	{
	case 1:
		req.channel = 2;
		break;
	case 2:
		req.channel = 1;
		break;
	default:
		req.channel = 1;
		break;
	}
	switch (pClient->GetZoneID())
	{
	case 3: // proc 1
	case 6: // proc 1
		req.zone = 1;
		break;
	case 1: // proc 2
		req.zone = 4;
		break;
	case 4: // proc 2
		req.zone = 1;
		break;
	case 2: // proc 3
	case 5: // proc 3
		req.zone = 4;
		break;

	default:
		req.zone = 6;
		break;
	}

	packet << req;

	pClient->SendEnqueuePacket(&packet);

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
}

bool st_Schedule_ReturnZone::DoInitRunSchedule(CClient* pClient)
{
	CPacket packet;
	st_CTS_ChangeZone req;
	// 로그인 스레드에서 벗어나기 위해 Zone 이동 패킷 전송
	req.zone = pClient->GetDeafultZoneID();
	req.channel = pClient->GetDefaultChannel();

	packet << req;

	pClient->SendEnqueuePacket(&packet);

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
}
