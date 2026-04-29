#include "CClient.h"
#include "CPacketProc.h"
#include "CUtill/CUtill.h"
#include "CDummy/CDummy.h"
#include "CDummy/DummyDef.h"
#include "Log/CLog.h"
static CPacketProc g_cPacketProc;

CClient::CClient(int Dummyid, int id)
{
	m_iManagementDummyID = Dummyid;
	m_iID = id;

	m_iServerID = 0;
	m_bLogin = false;
	m_iDefaultZoneID = 0;
	m_iDefaultChannel = 0;
	m_iZoneID = 0;
	m_iChannel = 0;

	m_pSchedule = nullptr;
	m_iWorkScheduleLoop = 0;
	m_iWorkScheduleRogress = 0;
	m_pWorkSchedule = nullptr;

	m_iSendDelay = 3 * 1000; // 3초
	m_iSendTime = 0;
}

void CClient::OnRecv(int type, CPacket& cPacket)
{
	m_PacketPool.Enqueue({ type, cPacket });
}

void CClient::Init(int channel, int zone, CSchedule* pSchedule)
{
	m_iDefaultChannel = channel;
	m_iDefaultZoneID = zone;
	SetSchedule(pSchedule);
}

void CClient::SetSchedule(CSchedule* pSchedule)
{
	m_pSchedule = pSchedule;
	m_iWorkScheduleRogress = 0;

	if (m_pWorkSchedule != nullptr)
		delete m_pWorkSchedule;

	SetFirstSchedule();
}

void CClient::SetWorkSchedule(st_Schedule* pSchedule)
{
	if (pSchedule == nullptr)
	{
		m_pWorkSchedule = nullptr;
		return;
	}

	m_pWorkSchedule = pSchedule;
	m_pWorkSchedule->DoInitRunSchedule(this);
}

void CClient::CheckSchedule()
{
	if (m_pWorkSchedule == nullptr)
		return;

	if (!m_pWorkSchedule->DoSchedule(this))
		return;

	NextSchedule();
}

void CClient::SetFirstSchedule()
{
	if (m_pSchedule == nullptr)
		return;

	st_Schedule* pPrevSchedule = m_pWorkSchedule;

	switch (m_pSchedule->GetSchedule(0))
	{
	case SCHEDULE_TYPE_LOGIN:
		SetWorkSchedule(new st_Schedule_Login());
		break;
	default:
		g_LogDummy.ELog("ERROR Invalid Schedule Type : %d", m_pSchedule->GetSchedule(0));
		break;
	}

	if (pPrevSchedule != nullptr)
		delete pPrevSchedule;
}

void CClient::NextSchedule()
{
	if (m_pSchedule == nullptr)
		return;

	m_iWorkScheduleRogress++;

	if (m_iWorkScheduleRogress >= m_pSchedule->GetSize())
	{
		m_iWorkScheduleLoop++;
		m_iWorkScheduleRogress = m_pSchedule->GetLogicScheduleIndex();
	}

	st_Schedule* pPrevSchedule = m_pWorkSchedule;

	switch (m_pSchedule->GetSchedule(m_iWorkScheduleRogress))
	{
	case SCHEDULE_TYPE_LOGIN:
		SetWorkSchedule(new st_Schedule_Login());
		break;
	case SCHEDULE_TYPE_LOGIN_CHANGE_ZONE:
		SetWorkSchedule(new st_Schedule_LoginChangeZone());
		break;
	case SCHEDULE_TYPE_CHANGE_ZONE:
		SetWorkSchedule(new st_Schedule_ChangeZone());
		break;
	case SCHEDULE_TYPE_RETURN_ZONE:
		SetWorkSchedule(new st_Schedule_ReturnZone());
		break;
	default:
		SetWorkSchedule(nullptr);
		break;
	}


	delete pPrevSchedule;
}

void CClient::Clear()
{
	m_iServerID = 0;
	m_bLogin = false;
}

void CClient::Update()
{
	RECV_JOB job;
	while (m_PacketPool.TryDequeue(job))
	{
		g_cPacketProc.DO_GAME_Proc(job.type, this, job.cPacket);
	}

	if (m_iSendTime + m_iSendDelay < GetTickCount())
	{
		SendPost();
		m_iSendTime = GetTickCount();
	}

	CheckSchedule();
}
