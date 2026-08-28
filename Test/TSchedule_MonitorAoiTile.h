#pragma once

#include "../Scheduling/CSchedule.h"
#include "../Stub/ProjectDefineStruct.h"
#include <vector>
#include <unordered_map>

class TSchedule_MonitorAoiTile : public CSchedule
{
public:
	TSchedule_MonitorAoiTile();
	~TSchedule_MonitorAoiTile() {};

private:
	int m_iWidth;
	int m_iHeight;
	int m_iTileW;
	int m_iTileH;
	int m_iTileCountW;
	int m_iTileCountH;
	int m_iIndex;
	
	std::vector<st_Vector3F> m_vecTileMonitorPositions;
	std::unordered_map<int, int> m_mapClientTileIndex;
public:
	void Init(int width, int height, int tileW, int tileH);
	const std::vector<st_Vector3F>& GetTileMonitorPositions() const { return m_vecTileMonitorPositions; }

	st_Vector3F GetPos(int clientID);
	int GetMointorClientID(st_Vector3F pos);
};

