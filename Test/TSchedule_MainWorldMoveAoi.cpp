#include "TSchedule_MainWorldMoveAoi.h"

#include "../Scheduling/ScheduleDefines.h"

#include <algorithm>
#include <chrono>

namespace
{
	int PositiveModulo(int value, int divisor)
	{
		const int result = value % divisor;
		return result < 0 ? result + divisor : result;
	}

	double GetSteadyTime()
	{
		return std::chrono::duration<double>(
			std::chrono::steady_clock::now().time_since_epoch()).count();
	}
}

TSchedule_MainWorldMoveAoi::TSchedule_MainWorldMoveAoi()
	: m_iWidth(0),
	m_iHeight(0),
	m_iGridCount(0),
	m_iTileW(0),
	m_iTileH(0),
	m_iTileCountW(0),
	m_iTileCountH(0),
	m_iExpectedClientCount(0),
	m_dPlacementSettleTime(5.0),
	m_dAoiSettleTime(2.0),
	m_dMoveInterval(3.0)
{
	m_iType = ESCHEDULE_TEST_TYPE::SCHEDULE_MAIN_WORLD_MOVE_AOI;
	m_vecSchedules.push_back(SCHEDULE_TYPE_MAIN_WORLD_MOVE_AOI);
}

void TSchedule_MainWorldMoveAoi::Init(
	int width,
	int height,
	int gridCount,
	int tileCountPerGrid)
{
	m_iWidth = width;
	m_iHeight = height;
	m_iGridCount = gridCount;

	const int gridW = width / gridCount;
	const int gridH = height / gridCount;
	m_iTileW = gridW / tileCountPerGrid;
	m_iTileH = gridH / tileCountPerGrid;
	m_iTileCountW = width / m_iTileW;
	m_iTileCountH = height / m_iTileH;
	m_iExpectedClientCount = GetTileCount();
}

int TSchedule_MainWorldMoveAoi::GetTileCount() const
{
	return m_iTileCountW * m_iTileCountH;
}

st_Vector3F TSchedule_MainWorldMoveAoi::GetStartPos(int clientID) const
{
	const int tileCount = GetTileCount();
	if (tileCount <= 0)
		return { 0.0f, 0.0f, 0.0f };

	// 연속된 Client ID 한 묶음은 MainWorld의 모든 Tile을 중복 없이 채운다.
	const int slot = PositiveModulo(clientID, tileCount);
	const int tileX = slot % m_iTileCountW;
	const int tileZ = slot / m_iTileCountW;

	return {
		static_cast<float>(tileX * m_iTileW) + static_cast<float>(m_iTileW) * 0.5f,
		0.0f,
		static_cast<float>(tileZ * m_iTileH) + static_cast<float>(m_iTileH) * 0.5f
	};
}

st_Vector3F TSchedule_MainWorldMoveAoi::GetMoveGoal(const st_Vector3F& currentPos) const
{
	int tileX = GetTileX(currentPos);
	int tileZ = GetTileZ(currentPos);

	// Grid 경계에 붙은 행은 Z축으로 교환해 실제 Grid 소유권 전환을 유도한다.
	const int rowsPerGrid = m_iTileCountH / m_iGridCount;
	const int rowInGrid = tileZ % rowsPerGrid;
	if (rowInGrid == rowsPerGrid - 1 && tileZ + 1 < m_iTileCountH)
	{
		++tileZ;
	}
	else if (rowInGrid == 0 && tileZ > 0)
	{
		--tileZ;
	}
	else
	{
		// 나머지 행도 한 Tile씩 움직이도록 X축 인접 Tile과 왕복한다.
		tileX += (tileX % 2 == 0) ? 1 : -1;
		tileX = std::clamp(tileX, 0, m_iTileCountW - 1);
	}

	return {
		static_cast<float>(tileX * m_iTileW) + static_cast<float>(m_iTileW) * 0.5f,
		0.0f,
		static_cast<float>(tileZ * m_iTileH) + static_cast<float>(m_iTileH) * 0.5f
	};
}

int TSchedule_MainWorldMoveAoi::GetTileX(const st_Vector3F& pos) const
{
	return m_iTileW > 0 ? static_cast<int>(pos.X) / m_iTileW : -1;
}

int TSchedule_MainWorldMoveAoi::GetTileZ(const st_Vector3F& pos) const
{
	return m_iTileH > 0 ? static_cast<int>(pos.Z) / m_iTileH : -1;
}

int TSchedule_MainWorldMoveAoi::GetManagementGrid(const st_Vector3F& pos) const
{
	if (m_iTileCountH <= 0)
		return -1;

	// GameServer의 MainWorld Grid 등록 규칙과 동일한 Z 대역 계산을 사용한다.
	return (GetTileZ(pos) * m_iGridCount) / m_iTileCountH;
}

bool TSchedule_MainWorldMoveAoi::IsValidPosition(const st_Vector3F& pos) const
{
	return pos.X >= 0.0f && pos.X < static_cast<float>(m_iWidth)
		&& pos.Z >= 0.0f && pos.Z < static_cast<float>(m_iHeight);
}

bool TSchedule_MainWorldMoveAoi::IsAoiInExpected(
	const st_Vector3F& start,
	const st_Vector3F& goal) const
{
	const int startX = GetTileX(start);
	const int startZ = GetTileZ(start);
	const int goalX = GetTileX(goal);
	const int goalZ = GetTileZ(goal);
	const int inX = goalX + (goalX - startX);
	const int inZ = goalZ + (goalZ - startZ);
	return inX >= 0 && inX < m_iTileCountW
		&& inZ >= 0 && inZ < m_iTileCountH;
}

bool TSchedule_MainWorldMoveAoi::IsAoiOutExpected(
	const st_Vector3F& start,
	const st_Vector3F& goal) const
{
	const int startX = GetTileX(start);
	const int startZ = GetTileZ(start);
	const int goalX = GetTileX(goal);
	const int goalZ = GetTileZ(goal);
	const int outX = startX - (goalX - startX);
	const int outZ = startZ - (goalZ - startZ);
	return outX >= 0 && outX < m_iTileCountW
		&& outZ >= 0 && outZ < m_iTileCountH;
}

void TSchedule_MainWorldMoveAoi::RecordPlacementReady(int channel)
{
	std::lock_guard<std::mutex> guard(m_CycleResultLock);
	int& readyCount = m_mapPlacementReadyCount[channel];
	++readyCount;
	if (readyCount == m_iExpectedClientCount)
	{
		m_mapReadyCycle[channel] = 0;
		m_mapNextCycleStartTime[channel] = GetSteadyTime() + m_dPlacementSettleTime;
	}
}

bool TSchedule_MainWorldMoveAoi::CanStartCycle(int channel, int cycle)
{
	std::lock_guard<std::mutex> guard(m_CycleResultLock);
	const auto readyIter = m_mapReadyCycle.find(channel);
	const auto timeIter = m_mapNextCycleStartTime.find(channel);
	return readyIter != m_mapReadyCycle.end()
		&& timeIter != m_mapNextCycleStartTime.end()
		&& readyIter->second == cycle
		&& GetSteadyTime() >= timeIter->second;
}

bool TSchedule_MainWorldMoveAoi::RecordCycleResult(
	int channel,
	int cycle,
	bool movePass,
	bool aoiPass,
	st_CycleSummary& summary)
{
	const long long key = (static_cast<long long>(channel) << 32)
		| static_cast<unsigned int>(cycle);

	std::lock_guard<std::mutex> guard(m_CycleResultLock);
	st_CycleResult& result = m_mapCycleResults[key];
	++result.CompletedCount;
	if (movePass)
		++result.MovePassCount;
	if (aoiPass)
		++result.AoiPassCount;

	if (result.CompletedCount < m_iExpectedClientCount)
		return false;

	summary.ClientCount = result.CompletedCount;
	summary.MovePassCount = result.MovePassCount;
	summary.AoiPassCount = result.AoiPassCount;
	m_mapCycleResults.erase(key);
	m_mapReadyCycle[channel] = cycle + 1;
	m_mapNextCycleStartTime[channel] = GetSteadyTime() + m_dMoveInterval;
	return true;
}
