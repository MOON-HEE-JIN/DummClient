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
}

void CClient::OnRecv(int type, CPacket& cPacket)
{
	m_PacketPool.Enqueue({ type, cPacket });
}

void CClient::SetSchedule(CSchedule* pSchedule)
{
	m_pSchedule = pSchedule;
	m_iWorkScheduleRogress = 0;

	if (m_pWorkSchedule != nullptr)
		delete m_pWorkSchedule;

	m_pWorkSchedule = nullptr;
}

void CClient::CheckSchedule()
{
	if (m_pWorkSchedule == nullptr)
		return;

	if (!m_pWorkSchedule->DoSchedule(this))
		return;

	NextSchedule();
}

void CClient::NextSchedule()
{
	if (m_pSchedule == nullptr)
		return;

	m_iWorkScheduleRogress++;

	if (m_iWorkScheduleRogress >= m_pSchedule->GetSize())
	{
		m_iWorkScheduleRogress = m_pSchedule->GetLogicScheduleIndex();
	}

	st_Schedule* pPrevSchedule = m_pWorkSchedule;

	switch (m_pSchedule->GetSchedule(m_iWorkScheduleRogress))
	{
	case SCHEDULE_TYPE_LOGIN:
		SetWorkSchedule(new st_Schedule_Login());
		break;
	case SCHEDULE_TYPE_CHANGE_ZONE:
		SetWorkSchedule(new st_Schedule_ChangeZone());
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

	CheckSchedule();
}
