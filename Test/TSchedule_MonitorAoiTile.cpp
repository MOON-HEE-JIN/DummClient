#include "TSchedule_MonitorAoiTile.h"

#include "../Scheduling/ScheduleDefines.h"

TSchedule_MonitorAoiTile::TSchedule_MonitorAoiTile()
{
	m_iType = ESCHEDULE_TEST_TYPE::SCHEDULE_MONITOR_AOI_TILE;
	m_vecSchedules.push_back(SCHEDULE_TYPE_MONITOR_AOI_TILE);
	m_vecSchedules.push_back(SCHEDULE_TYPE_DELAY);
}

void TSchedule_MonitorAoiTile::Init(int width, int height, int tileW, int tileH)
{
	m_iWidth = width;
	m_iHeight = height;
	m_iTileW = tileW;
	m_iTileH = tileH;
	m_iIndex = 0;
	
	m_iTileCountW = m_iWidth / m_iTileW;
	m_iTileCountH = m_iHeight / m_iTileH;

	m_vecTileMonitorPositions.clear();
	// ex. World = 1024 * 1024 Tile = 64 * 64

	for (int h = 0; h < m_iTileCountH; h++)
	{
		for(int w = 0; w < m_iTileCountW; w++)
		{
			st_Vector3F pos;
			pos.X = static_cast<float>(w * m_iTileW) + (static_cast<float>(m_iTileW) * 0.5f);
			pos.Y = 0.0f;
			pos.Z = static_cast<float>(h * m_iTileH) + (static_cast<float>(m_iTileH) * 0.5f);
			m_vecTileMonitorPositions.push_back(pos);
		}
	}
}

st_Vector3F TSchedule_MonitorAoiTile::GetPos(int clientID)
{
	int index = InterlockedIncrement(reinterpret_cast<LONG*>(&m_iIndex)) - 1;
	index = index % static_cast<int>(m_vecTileMonitorPositions.size());
	m_mapClientTileIndex[clientID] = index;
	return m_vecTileMonitorPositions[index];
}

int TSchedule_MonitorAoiTile::GetMointorClientID(st_Vector3F pos)
{
	if (m_mapClientTileIndex.empty())
		return -1;

	int index = 0;
	int tileW = pos.X / static_cast<float>(m_iTileW);
	int tileH = pos.Z / static_cast<float>(m_iTileH);
	index = tileH * m_iTileCountW + tileW;
	
	if (m_mapClientTileIndex.find(index) == m_mapClientTileIndex.end())
		return -1;
	return m_mapClientTileIndex[index];
}
