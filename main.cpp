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
	
	// 종합적 테스트
	// 1. 각 Zone LoopBack 고정
	// 2. 각 Zone LoopBack 이동 LoopBack 반복
	// 3. 각 Zone LoopBack 연결 해제, 재연결 LoopBack
	// Loop Back
	for (int channel = 0; channel < 2; channel++)
	{
		for (int zone = 2; zone < 7; zone++)
		{
			g_DummyManager.CreateDummy(channel, zone, 200, ESCHEDULE_TEST_TYPE::SCHEDULE_LOOPBACK);
			g_DummyManager.CreateDummy(channel, zone, 10, ESCHEDULE_TEST_TYPE::SCHEDULE_LOOPBACK_CHANGEZONE);
			g_DummyManager.CreateDummy(channel, zone, 10, ESCHEDULE_TEST_TYPE::SCHEDULE_LOOPBACK_DISRECONNECT);
			Sleep(1000); // Dummy 생성 대기
		}
	}
	// MainWorld는 Tile당 한 명을 배치한 뒤 인접 Tile/Grid를 왕복하며 Move/AOI를 검증한다.
	g_DummyManager.CreateDummy(0, 1, 16 * 16, ESCHEDULE_TEST_TYPE::SCHEDULE_MAIN_WORLD_MOVE_AOI);
	g_DummyManager.CreateDummy(1, 1, 16 * 16, ESCHEDULE_TEST_TYPE::SCHEDULE_MAIN_WORLD_MOVE_AOI);

	//g_DummyManager.CreateDummy(0, 1, 16 * 16, ESCHEDULE_TEST_TYPE::SCHEDULE_MONITOR_AOI_TILE);
	//g_DummyManager.CreateDummy(0, 1, 50, ESCHEDULE_TEST_TYPE::SCHEDULE_LOOPBACK);
	//g_DummyManager.CreateDummy(1, 1, 150, ESCHEDULE_TEST_TYPE::SCHEDULE_LOOPBACK);
	//g_DummyManager.CreateDummy(2, 1, 150, ESCHEDULE_TEST_TYPE::SCHEDULE_LOOPBACK);

	WaitThread();
	return 0;
}
