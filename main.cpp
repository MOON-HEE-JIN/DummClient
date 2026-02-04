#include "NetWork/CNetWork.h"
#include "NetWork/CSession.h"

#include "Stub/PacketEnumDef.h"

#include "CClient.h"
#include "CPacketProc.h"
#include "CDummy/CDummy.h"
void main()
{
	
	StartThread();

	g_DummyManager.StartDummyClients(1000);
	// 서버 쪽 연결 로그 끝나면 보내기
	// 로그 쉽게 보기 위해서 약간의 딜레이
	Sleep(1000);
	g_DummyManager.SendLoopbackPackets();
	while (true)
	{
		g_DummyManager.Update();
		Sleep(100);
	}

	WaitThread();
}
