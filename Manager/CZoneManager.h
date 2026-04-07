#pragma once

#include <vector>
#include <unordered_map>
#include "../Zone/ZoneDefines.h"
#include "../Manager/CBinFileManager.h"
#include "../CGrid/CGrid.h"
class CZoneManager
{
public:
	CZoneManager();
	~CZoneManager();

private:
	std::unordered_map<int, st_IDX> m_mapZoneIDX;
	std::unordered_map<int, CGrid*> m_mapZoneGrid;

public:
	const std::unordered_map<int, st_IDX>& GetZoneIDXMap() { return m_mapZoneIDX; }


	friend class CBinFileManager;
};

extern CZoneManager g_ZoneManager;
