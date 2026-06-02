#include "TSchedule_PlaceMainworld.h"

#include "../Scheduling/ScheduleDefines.h"
TSchedule_PlaceMainWorld::TSchedule_PlaceMainWorld()
{
	m_iType = ESCHEDULE_TEST_TYPE::SCHEDULE_MAIN_WORLD;

	m_vecSchedules.push_back(SCHEDULE_TYPE_TELEPORT);
	m_vecSchedules.push_back(SCHEDULE_TYPE_DELAY);
}

void TSchedule_PlaceMainWorld::Init(int width, int height, int gridcount, int tilecount)
{
	m_iGridW = width / gridcount;
	m_iGridH = height / gridcount;

	m_iTileW = m_iGridW / tilecount;
	m_iTileH = m_iGridH / tilecount;

	int countx = width / m_iTileW;
	int county = height / m_iTileH;

	for (int Y = 0; Y < county; Y++)
	{
		for (int X = 0; X < countx; X++)
		{
			m_vecPos.push_back({(float)(m_iTileW * X) +(m_iTileW / 2), 0, (float)(m_iTileH * Y) +(m_iTileH / 2) });
		}
	}
}

st_Vector3F TSchedule_PlaceMainWorld::GetPos()
{
	st_Vector3F ret = m_vecPos[index++];
	if (index >= m_vecPos.size())
		index = 0;
	return ret;
}
