#include "NetWork/CNetWork.h"
#include "NetWork/CSession.h"

#include "Stub/PacketEnumDef.h"

#include "CClient.h"
#include "CPacketProc.h"

void main()
{
	
	StartThread();
	CClient* pClient = new CClient;
	pClient->Connect("127.0.0.1", 7799, (HANDLE)GetCICPPort());
	st_CTS_LoopBack data;
	data.data = pClient->GetLoopbackData();
	
	CPacket pRes;
	pRes << data;
	pClient->SendPacket(0, &pRes);

	WaitThread();
}