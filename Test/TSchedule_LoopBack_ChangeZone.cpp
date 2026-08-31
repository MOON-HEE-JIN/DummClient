#include "TSchedule_LoopBack_ChangeZone.h"
#include "../Scheduling/ScheduleDefines.h"

TSchedule_LoopBack_ChangeZone::TSchedule_LoopBack_ChangeZone()
{
	m_iType = ESCHEDULE_TEST_TYPE::SCHEDULE_LOOPBACK_CHANGEZONE;

	for(int i = 0; i < 50; i++)
		m_vecSchedules.push_back(SCHEDULE_TYPE_LOOPBACK);

	m_vecSchedules.push_back(SCHEDULE_TYPE_CHANGE_ZONE);
}
