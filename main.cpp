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
	printf("=== Send Start ===\n");
	g_DummyManager.SendLoopbackPackets();
	while (true)
	{
		g_DummyManager.Update();
		Sleep(100);
	}

	WaitThread();
}
