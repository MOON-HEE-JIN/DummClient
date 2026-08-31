#pragma once

#include "../Scheduling/CSchedule.h"
#include "../Stub/ProjectDefineStruct.h"

#include <map>
#include <mutex>

class TSchedule_MainWorldMoveAoi : public CSchedule
{
public:
	struct st_CycleSummary
	{
		int ClientCount = 0;
		int MovePassCount = 0;
		int AoiPassCount = 0;
	};

	TSchedule_MainWorldMoveAoi();
	~TSchedule_MainWorldMoveAoi() {};

private:
	struct st_CycleResult
	{
		int CompletedCount = 0;
		int MovePassCount = 0;
		int AoiPassCount = 0;
	};

	int m_iWidth;
	int m_iHeight;
	int m_iGridCount;
	int m_iTileW;
	int m_iTileH;
	int m_iTileCountW;
	int m_iTileCountH;
	int m_iExpectedClientCount;
	double m_dPlacementSettleTime;
	double m_dAoiSettleTime;
	double m_dMoveInterval;

	std::mutex m_CycleResultLock;
	std::map<long long, st_CycleResult> m_mapCycleResults;
	std::map<int, int> m_mapPlacementReadyCount;
	std::map<int, int> m_mapReadyCycle;
	std::map<int, double> m_mapNextCycleStartTime;

public:
	void Init(int width, int height, int gridCount, int tileCountPerGrid);

	int GetTileCount() const;
	st_Vector3F GetStartPos(int clientID) const;
	st_Vector3F GetMoveGoal(const st_Vector3F& currentPos) const;

	int GetTileX(const st_Vector3F& pos) const;
	int GetTileZ(const st_Vector3F& pos) const;
	int GetManagementGrid(const st_Vector3F& pos) const;
	bool IsValidPosition(const st_Vector3F& pos) const;
	bool IsAoiInExpected(const st_Vector3F& start, const st_Vector3F& goal) const;
	bool IsAoiOutExpected(const st_Vector3F& start, const st_Vector3F& goal) const;

	double GetAoiSettleTime() const { return m_dAoiSettleTime; }

	void RecordPlacementReady(int channel);
	bool CanStartCycle(int channel, int cycle);

	bool RecordCycleResult(
		int channel,
		int cycle,
		bool movePass,
		bool aoiPass,
		st_CycleSummary& summary);
};
