#pragma once

#include <vector>

class CSchedule
{
public:
	CSchedule();
	~CSchedule() {};

protected:
	int m_iType;
	int m_iLogicScheduleIndex;
	std::vector<int> m_vecSchedules;

public:
	int GetSize() const { return static_cast<int>(m_vecSchedules.size()); }
	int GetSchedule(int index) const { return m_vecSchedules[index]; }
	int GetLogicScheduleIndex() const { return m_iLogicScheduleIndex; }
	int GetType() { return m_iType; }
	virtual void casting() {};
};