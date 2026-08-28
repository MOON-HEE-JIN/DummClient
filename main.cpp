#include "NetWork/CNetWork.h"
#include "NetWork/CSession.h"

#include "Stub/PacketEnumDef.h"

#include "CClient.h"
#include "CPacketProc.h"
#include "CDummy/CDummyManager.h"
#include "Test/TSchedule_Change_Zone.h"
std::vector<CDummy*> vecDummy;
int main()
{
	StartThread();
	Sleep(1000); // NetWork Thread 시작 대기
	
	for (int channel = 0; channel < 2; channel++)
	{
		for (int zone = 2; zone < 7; zone++)
		{
			//g_DummyManager.CreateDummy(channel, zone, 50, ESCHEDULE_TEST_TYPE::SCHEDULE_MOVE);
			//g_DummyManager.CreateDummy(channel, zone, 200, ESCHEDULE_TEST_TYPE::SCHEDULE_LOOPBACK);
			//Sleep(1000); // Dummy 생성 대기
		}
	}
	
	g_DummyManager.CreateDummy(0, 1, 16 * 16, ESCHEDULE_TEST_TYPE::SCHEDULE_MONITOR_AOI_TILE);
	//g_DummyManager.CreateDummy(0, 1, 16 * 16, ESCHEDULE_TEST_TYPE::SCHEDULE_MAIN_WORLD);
	//g_DummyManager.CreateDummy(0, 1, 50, ESCHEDULE_TEST_TYPE::SCHEDULE_LOOPBACK);
	//g_DummyManager.CreateDummy(1, 1, 150, ESCHEDULE_TEST_TYPE::SCHEDULE_LOOPBACK);
	//g_DummyManager.CreateDummy(2, 1, 150, ESCHEDULE_TEST_TYPE::SCHEDULE_LOOPBACK);

	WaitThread();
	return 0;
}
