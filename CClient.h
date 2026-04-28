#pragma once
#include "NetWork/CSession.h"
class CClient : public CSession
{
public:
	CClient(int Dummyid, int id);
	~CClient() {}

	// CSession 에서 상속
	void OnRecv(int type, CPacket& cPacket) override;

	void DisConnect()
	{
		CloseSocket();
	}
	void ReConnect(const char IP[16], unsigned short Port, HANDLE cicp);
	void Update() {};

private:
	int m_iManagementDummyID;		// 관리하는 CDummy ID
	int m_iID;
	int m_iServerID;

};