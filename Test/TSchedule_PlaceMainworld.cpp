#include "TSchedule_PlaceMainworld.h"

#include "../Scheduling/ScheduleDefines.h"
#include <chrono>0000000

namespace
{
	long long GetSteadyTimeMs()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now().time_since_epoch()).count();
	}
}
TSchedule_PlaceMainWorld::TSchedule_PlaceMainWorld()
	: m_iWidth(0),
	m_iHeight(0),
	m_iGridCount(0),
	m_iGridW(0),
	m_iGridH(0),
	m_iTileW(0),
	m_iTileH(0),
	m_iFirstClientID(0),
	m_iExpectedClientCount(0),
	m_iTeleportIntervalMs(3000),
	m_iCurrentCycle(0),
	m_iCompletedClientCount(0),
	m_iTeleportPassCount(0),
	m_iNextCycleStartTimeMs(0)
{
	m_iType = ESCHEDULE_TEST_TYPE::SCHEDULE_MAIN_WORLD;

	// 초기 배치와 이후 주기 이동을 모두 TELEPORT 패킷만으로 수행한다.
	m_vecSchedules.push_back(SCHEDULE_TYPE_MAIN_WORLD_TELEPORT);
}

void TSchedule_PlaceMainWorld::Init(int width, int height, int gridcount, int tilecount)
{
	m_iWidth = width;
	m_iHeight = height;
	m_iGridCount = gridcount;

	m_iGridW = width / gridcount;
	m_iGridH = height / gridcount;

	m_iTileW = m_iGridW / tilecount;
	m_iTileH = m_iGridH / tilecount;
}

void TSchedule_PlaceMainWorld::PrepareRun(int firstClientID, int clientCount)
{
	m_iFirstClientID = firstClientID;
	m_iExpectedClientCount = clientCount;
	m_iCurrentCycle.store(0);
	m_iCompletedClientCount.store(0);
	m_iTeleportPassCount.store(0);
	m_iNextCycleStartTimeMs.store(0);
}

int TSchedule_PlaceMainWorld::GetTileCount() const
{
	if (m_iTileW <= 0 || m_iTileH <= 0)
		return 0;

	return (m_iWidth / m_iTileW) * (m_iHeight / m_iTileH);
}

st_Vector3F TSchedule_PlaceMainWorld::GetStartPos(int clientID) const
{
	return GetCyclePos(clientID, 0);
}

st_Vector3F TSchedule_PlaceMainWorld::GetCyclePos(int clientID, int cycle) const
{
	const int tileCountW = m_iWidth / m_iTileW;
	const int tileCountH = m_iHeight / m_iTileH;
	const int tileCount = GetTileCount();
	if (tileCountW <= 0 || tileCountH <= 0 || tileCount <= 0)
		return { 0.0f, 0.0f, 0.0f };

	// 매 주기마다 모든 행을 한 칸 순환시켜 목적지 타일도 항상 1:1로 유지한다.
	const int slot = (clientID - m_iFirstClientID) % tileCount;
	const int tileX = slot % tileCountW;
	const int startTileZ = slot / tileCountW;
	const int cycleOffset = cycle % tileCountH;
	const int tileZ = (startTileZ + cycleOffset) % tileCountH;

	return {
		static_cast<float>(tileX * m_iTileW) + (static_cast<float>(m_iTileW) * 0.5f),
		0.0f,
		static_cast<float>(tileZ * m_iTileH) + (static_cast<float>(m_iTileH) * 0.5f)
	};
}

bool TSchedule_PlaceMainWorld::CanStartCycle(int cycle) const
{
	return m_iCurrentCycle.load() == cycle
		&& GetSteadyTimeMs() >= m_iNextCycleStartTimeMs.load();
}

bool TSchedule_PlaceMainWorld::RecordCycleResult(int cycle, bool teleportPass, int& passCount)
{
	if (cycle != m_iCurrentCycle.load())
		return false;

	if (teleportPass)
		m_iTeleportPassCount.fetch_add(1);

	const int completed = m_iCompletedClientCount.fetch_add(1) + 1;
	if (completed < m_iExpectedClientCount)
		return false;

	passCount = m_iTeleportPassCount.load();
	m_iCompletedClientCount.store(0);
	m_iTeleportPassCount.store(0);
	m_iNextCycleStartTimeMs.store(GetSteadyTimeMs() + m_iTeleportIntervalMs);
	m_iCurrentCycle.store(cycle + 1);
	return true;
}

int TSchedule_PlaceMainWorld::GetTileX(const st_Vector3F& pos) const
{
	return static_cast<int>(pos.X) / m_iTileW;
}

int TSchedule_PlaceMainWorld::GetTileZ(const st_Vector3F& pos) const
{
	return static_cast<int>(pos.Z) / m_iTileH;
}

int TSchedule_PlaceMainWorld::GetManagementGrid(const st_Vector3F& pos) const
{
	return GetTileZ(pos) % m_iGridCount;
}
