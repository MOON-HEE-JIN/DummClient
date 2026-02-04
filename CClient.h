#pragma once
#include "NetWork/CSession.h"
class CClient : public CSession
{
public:
	CClient() {}
	~CClient() {}

	// CSession을(를) 통해 상속됨
	void OnRecv(int type, CPacket& cPacket) override;

};