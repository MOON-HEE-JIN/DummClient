#pragma once

#include "Stub/CLIENT_STUB.h"
#include "Stub/CLIENT_STUB.cpp"

#include "CClient.h"
#include "CUtill/CPacket.h"
#include "Stub/PacketEnumDef.h"

class CClient;

class CPacketProc : public Stub<CClient, CPacket>
{
public:
	CPacketProc() {};
	~CPacketProc() {};
private:
	// Stub을(를) 통해 상속됨
	int DO_GAME_LOOPBACK(CClient* pTarget, CPacket& pReqPacket);
	
	int DO_ERROR_PACKET(CClient* pTarget, CPacket& pReqPacket);
	int DO_ERROR_RESULT(CClient* pTarget, int ret, int type);
};