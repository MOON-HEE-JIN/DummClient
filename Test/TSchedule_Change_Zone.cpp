#include "TSchedule_Change_Zone.h"
#include "../Scheduling/ScheduleDefines.h"

TSchedule_Change_Zone::TSchedule_Change_Zone()
{
	m_iType = SCHEDULE_TEST_TYPE::SCHEDULE_RETURN_ZONE;

	m_vecSchedules.push_back(SCHEDULE_TYPE_CHANGE_ZONE);		// 3: Zone 이동 대기
	m_vecSchedules.push_back(SCHEDULE_TYPE_RETURN_ZONE);
}
