#include "TSchedule_LoopBack_DisReConnect.h"
#include "../Scheduling/ScheduleDefines.h"

TSchedule_LoopBack_DisReConnect::TSchedule_LoopBack_DisReConnect()
{
	m_iType = ESCHEDULE_TEST_TYPE::SCHEDULE_LOOPBACK_DISRECONNECT;

	for (int i = 0; i < 10; i++)
		m_vecSchedules.push_back(SCHEDULE_TYPE_LOOPBACK);

	m_vecSchedules.push_back(SCHEDULE_TYPE_DISCONNECT);
	m_vecSchedules.push_back(SCHEDULE_TYPE_RECONNECT);
	m_vecSchedules.push_back(SCHEDULE_TYPE_LOGIN);
	m_vecSchedules.push_back(SCHEDULE_TYPE_LOGIN_CHANGE_ZONE);
}
