#pragma once
#include "../Manager/CBinFileManager.h"
#include "../Stub/ProjectDefineStruct.h"
#include "GridDefine.h"

class CGrid
{
private:
	int m_iZoneID;
	float m_fVoxelSize;

	st_Vector3F m_stOrigin;
	st_Vector3F m_stExtent[2];
	//st_Vector3D m_stGridSize;

	std::unordered_map<BlockCoord, BlockData, BlockCoordHasher> Blocks;
public:

	friend class CBinFileManager;
};