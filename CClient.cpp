#include "CClient.h"
#include "CPacketProc.h"


static CPacketProc g_cPacketProc;

void CClient::OnRecv(int type, CPacket& cPacket)
{
	printf("CClient::OnRecv Type : %d \n", type);
	g_cPacketProc.DO_GAME_Proc(type, this, cPacket);
}
