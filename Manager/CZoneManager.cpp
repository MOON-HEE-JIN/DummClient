#include "CZoneManager.h"

#include "../GameServerDef.h"
#include "../Log/CLog.h"
#include "../Stub/GameClient.PacketEnumDef.g.h"
#include "../Stub/GameClient.EnumDef.g.h"

#include <sstream>

#include "../Zone/CBinZoneIdx.h"
#include "../Zone/CBinZone.h"

CZoneManager::CZoneManager()
{

}

CZoneManager::~CZoneManager()
{
	
}

CZoneManager g_ZoneManager;
