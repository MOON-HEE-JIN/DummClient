#include "NetWork/CNetWork.h"
#include "NetWork/CSession.h"

#include "Stub/PacketEnumDef.h"

#include "CClient.h"
#include "CPacketProc.h"
#include "CDummy/CDummy.h"
void main()
{
	
	StartThread();

	g_DummyManager.StartDummyClients();
	printf("== Delay Connect ===");
	Sleep(5 * 1000);
	printf("=== Send Start ===\n");
	while (true)
	{
		g_DummyManager.Update();
		Sleep(100);
	}

	WaitThread();
}
