#include "NetWork/CNetWork.h"
#include "NetWork/CSession.h"

#include "Stub/PacketEnumDef.h"

#include "CClient.h"
#include "CPacketProc.h"
#include "CDummy/CDummy.h"
#include "Test/TSchedule_Change_Zone.h"
std::vector<CDummy*> vecDummy;
void main()
{
	StartThread();
	Sleep(1000); // NetWork Thread 시작 대기
	// 임시
	int DummyID = 1;

	for (int channel = 0; channel <= 1; channel++)
	{
		for (int zone = 1; zone <= 6; zone++)
		{
			CDummy* pDummy = new CDummy(DummyID++, "127.0.0.1", 7799, 50);
			vecDummy.push_back(pDummy);
			pDummy->Init(channel, zone, new TSchedule_Change_Zone());
		}
	}

	for (CDummy* pDummy : vecDummy)
	{
		pDummy->Start();
	}

	for (CDummy* pDummy : vecDummy)
	{
		pDummy->Wait();
	}

	WaitThread();
}