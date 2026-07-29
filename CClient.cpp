#include "CClient.h"
#include "CPacketProc.h"
#include "CUtill/CUtill.h"
#include "CDummy/CDummy.h"
#include "CDummy/DummyDef.h"
#include "Log/CLog.h"
#include "CDummy/CDummyManager.h"

#include "Test/TSchedule_PlaceMainworld.h"

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

	m_iSendDelay = CUtil::Random(5, 10) * 100;
	m_iSendTime = 0;

	m_iCompleteScheduleCount.store(0);
	m_dLatencyTime.store(0);

	m_fMoveSpeed = 5.0f;
	m_stPos = { 0.0f, 0.0f, 0.0f };
	m_ddRecvLoopData = -1;
	m_iTeleportResult = -1;
	m_iAoiInTransitionCount = 0;
	m_iAoiOutTransitionCount = 0;

	m_eState = ESTATE::IDEL;
	
}

CClient::~CClient()
{
	delete m_pWorkSchedule;
	m_pWorkSchedule = nullptr;
}

void CClient::OnRecv(int type, CPacket& cPacket, LONGLONG recvtime)
{
	const LONGLONG sendtime = PopSendTime(type);

	m_queue.Push(RECV_JOB(type, std::move(cPacket)));
	
	if (sendtime == -1)
		return;

	const double time = static_cast<double>(recvtime - sendtime) * 1000.0
		/ static_cast<double>(freq.QuadPart); // ms
	m_dLatencyTime.store(time);
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

	m_pWorkSchedule = nullptr;

	SetFirstSchedule();
}

int CClient::GetScheduleType()
{
	if (m_pSchedule == nullptr)
		return 0;
	return m_pSchedule->GetType();
}

void CClient::SetWorkSchedule(st_Schedule* pSchedule)
{
	if (pSchedule == nullptr)
	{
		if (m_pWorkSchedule != nullptr)
			delete m_pWorkSchedule;
		m_pWorkSchedule = nullptr;
		return;
	}

	// 기존 스케쥴 제거
	if (m_pWorkSchedule != nullptr)
		delete m_pWorkSchedule;

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
	if (m_pSchedule == nullptr || m_pSchedule->GetSize() <= 0)
		return;

	switch (m_pSchedule->GetSchedule(0))
	{
	case SCHEDULE_TYPE_LOGIN:
		SetWorkSchedule(new st_Schedule_Login());
		break;
	default:
		g_LogDummy.ELog("ERROR Invalid Schedule Type : %d", m_pSchedule->GetSchedule(0));
		break;
	}
}

void CClient::NextSchedule()
{
	if (m_pSchedule == nullptr)
		return;

	AddCompleteScheduleCount();
	
	m_iWorkScheduleRogress++;

	if (m_iWorkScheduleRogress >= m_pSchedule->GetSize())
	{
		m_iWorkScheduleLoop++;
		m_iWorkScheduleRogress = m_pSchedule->GetLogicScheduleIndex();
	}

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
	case SCHEDULE_TYPE_MOVE_START:
		SetWorkSchedule(new st_Schedule_MoveStart());
		break;
	case SCHEDULE_TYPE_LOOPBACK:
		SetWorkSchedule(new st_Schedule_LoopBack());
		break;
	case SCHEDULE_TYPE_TELEPORT:
	{
		st_Schedule_Teleport* pSchedule = new st_Schedule_Teleport();
		
		TSchedule_PlaceMainWorld* pS = dynamic_cast<TSchedule_PlaceMainWorld*>(g_DummyManager.GetSchedule(ESCHEDULE_TEST_TYPE::SCHEDULE_MAIN_WORLD));
		
		if (pS)
			pSchedule->GoalPos = pS->GetStartPos(m_iID);

		SetWorkSchedule(pSchedule);
	}
		break;
	case SCHEDULE_TYPE_MAIN_WORLD_TELEPORT:
	{
		TSchedule_PlaceMainWorld* pS = dynamic_cast<TSchedule_PlaceMainWorld*>(
			g_DummyManager.GetSchedule(ESCHEDULE_TEST_TYPE::SCHEDULE_MAIN_WORLD));
		if (pS == nullptr)
		{
			g_LogDummy.ELog("ERROR MainWorld Schedule Client[%d]", m_iID);
			SetWorkSchedule(nullptr);
			break;
		}

		SetWorkSchedule(new st_Schedule_MainWorldTeleport(pS));
	}
		break;
	case SCHEDULE_TYPE_DELAY:
		SetWorkSchedule(new st_Schedule_Delay());
		break;
	default:
		SetWorkSchedule(nullptr);
		break;
	}
}

void CClient::Clear()
{
	m_iServerID = 0;
	m_bLogin = false;
	m_iTeleportResult = -1;
	m_visiblePlayers.clear();
	ResetAoiTransitionCount();
}

void CClient::AddVisiblePlayer(int id)
{
	if (id != m_iServerID)
	{
		if (m_visiblePlayers.insert(id).second)
			++m_iAoiInTransitionCount;
	}
}

void CClient::RemoveVisiblePlayer(int id)
{
	if (m_visiblePlayers.erase(id) > 0)
		++m_iAoiOutTransitionCount;
}

void CClient::ResetAoiTransitionCount()
{
	m_iAoiInTransitionCount = 0;
	m_iAoiOutTransitionCount = 0;
}

void CClient::Update()
{
	std::vector<RECV_JOB> jobs;
	m_queue.PopVector(jobs);

	for (RECV_JOB& job : jobs)
	{
		g_cPacketProc.DO_GAME_Proc(job.type, this, job.cPacket);
	}

	const ULONGLONG now = GetTickCount64();
	if (m_iSendTime + static_cast<ULONGLONG>(m_iSendDelay) < now)
	{
		SendPost();
		m_iSendTime = now;
	}

	CheckSchedule();
}
