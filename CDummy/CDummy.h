#pragma once

#include "../CClient.h"
#include <vector>

class CDummy
{
public:
	CDummy();
	~CDummy();


	void Update();
	void StartDummyClients(int nClientCount);

	void SendLoopbackPackets();
private:
	std::vector<CClient*> m_DummyClients;
};