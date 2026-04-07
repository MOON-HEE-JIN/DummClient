#include "NetWork/CNetWork.h"
#include "NetWork/CSession.h"

#include "Stub/PacketEnumDef.h"

#include "CClient.h"
#include "CPacketProc.h"
#include "CDummy/CDummy.h"
#include "Manager/CBinFileManager.h"

void main()
{	
	StartThread();

	CBinFileManager bin;

	bin.LoadBinFiles();

	g_DummyManager.StartDummyClients();
	printf("=== Send Start ===\n");
	while (true)
	{
		g_DummyManager.Update();
		Sleep(100);
	}

	WaitThread();
}
