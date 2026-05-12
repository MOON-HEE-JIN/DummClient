#include "NetWork/CNetWork.h"
#include "NetWork/CSession.h"

#include "Stub/PacketEnumDef.h"

#include "CClient.h"
#include "CPacketProc.h"
#include "CDummy/CDummyManager.h"
#include "Test/TSchedule_Change_Zone.h"
std::vector<CDummy*> vecDummy;
void main()
{
	StartThread();
	Sleep(1000); // NetWork Thread 시작 대기
	
	for (int channel = 0; channel <= 1; channel++)
	{
		for (int zone = 1; zone <= 6; zone++)
		{
			g_DummyManager.CreateDummy(channel, zone, 50, ESCHEDULE_TEST_TYPE::SCHEDULE_MOVE);
		}
	}

	WaitThread();
}