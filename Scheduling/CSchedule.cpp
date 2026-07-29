#include "CSchedule.h"
#include "ScheduleDefines.h"

CSchedule::CSchedule()
{
	m_vecSchedules.push_back(SCHEDULE_TYPE_LOGIN);				// 1: 로그인 응답 대기
	m_vecSchedules.push_back(SCHEDULE_TYPE_LOGIN_CHANGE_ZONE);		// 2: 로그인 스레드 에서 벗어나기

	// Login 완료후 로직 담당 스케줄
	m_iLogicScheduleIndex = static_cast<int>(m_vecSchedules.size());
}
