#include "TSchedule_LoopBack.h"
#include "../Scheduling/ScheduleDefines.h"

TSchedule_LoopBack::TSchedule_LoopBack()
{
	m_iType = ESCHEDULE_TEST_TYPE::SCHEDULE_RETURN_ZONE;

	m_vecSchedules.push_back(SCHEDULE_TYPE_LOOPBACK);		
}
