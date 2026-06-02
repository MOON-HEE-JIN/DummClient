#pragma once

#include "../Scheduling/CSchedule.h"
#include "../Stub/ProjectDefineStruct.h"
#include <vector>
class TSchedule_PlaceMainWorld : public CSchedule
{
public:
	TSchedule_PlaceMainWorld();
	~TSchedule_PlaceMainWorld() {};

private:
	std::vector<st_Vector3F> m_vecPos;
	int index = 0;

	int m_iGridW;
	int m_iGridH;
	int m_iTileW;
	int m_iTileH;

public:
	void Init(int width, int height, int gridcount, int tilecount);
	st_Vector3F GetPos();
};