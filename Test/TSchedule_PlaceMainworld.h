#pragma once

#include "../Scheduling/CSchedule.h"
#include "../Stub/ProjectDefineStruct.h"
#include <atomic>

class TSchedule_PlaceMainWorld : public CSchedule
{
public:
	TSchedule_PlaceMainWorld();
	~TSchedule_PlaceMainWorld() {};

private:
	int m_iWidth;
	int m_iHeight;
	int m_iGridCount;
	int m_iGridW;
	int m_iGridH;
	int m_iTileW;
	int m_iTileH;

	int m_iFirstClientID;
	int m_iExpectedClientCount;
	int m_iTeleportIntervalMs;
	std::atomic<int> m_iCurrentCycle;
	std::atomic<int> m_iCompletedClientCount;
	std::atomic<int> m_iTeleportPassCount;
	std::atomic<long long> m_iNextCycleStartTimeMs;

public:
	void Init(int width, int height, int gridcount, int tilecount);
	void PrepareRun(int firstClientID, int clientCount);

	int GetTileCount() const;
	st_Vector3F GetStartPos(int clientID) const;
	st_Vector3F GetCyclePos(int clientID, int cycle) const;

	bool CanStartCycle(int cycle) const;
	bool RecordCycleResult(int cycle, bool teleportPass, int& passCount);
	int GetExpectedClientCount() const { return m_iExpectedClientCount; }

	int GetTileX(const st_Vector3F& pos) const;
	int GetTileZ(const st_Vector3F& pos) const;
	int GetManagementGrid(const st_Vector3F& pos) const;
};
