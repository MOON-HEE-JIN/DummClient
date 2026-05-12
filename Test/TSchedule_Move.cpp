#include "TSchedule_Move.h"
#include "../Scheduling/ScheduleDefines.h"

TSchedule_Move::TSchedule_Move()
{
	m_iType = ESCHEDULE_TEST_TYPE::SCHEDULE_MOVE;

	m_vecSchedules.push_back(SCHEDULE_TYPE_MOVE_START);
}
