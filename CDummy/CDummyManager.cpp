#include "CDummyManager.h"
#include "../Test/TSchedule_Change_Zone.h"
#include "../Test/TSchedule_Move.h"

CDummyManager g_DummyManager;

CDummyManager::CDummyManager()
{
    m_iDummyID = 0;
    m_iClientID = 0;

    memcpy(m_szIP, "127.0.0.1", 10);
    m_sPort = 7799;

    m_vecSchedules.push_back(new TSchedule_Change_Zone());
    m_vecSchedules.push_back(new TSchedule_Move());

    InitializeCriticalSection(&cs);
}

CDummyManager::~CDummyManager()
{
    std::map<int, CDummy*>::iterator biter = m_mapDummys.begin();
    std::map<int, CDummy*>::iterator eiter = m_mapDummys.end();
    for (biter; biter != eiter; ++biter)
    {
        biter->second->Wait();
    }
}

bool CDummyManager::CreateDummy(int channel, int zone, int count, int scheduleType)
{
    int ID = m_iDummyID++;
    CDummy* pDummy = new CDummy(ID, m_szIP, m_sPort, count, m_iClientID);
    m_iClientID += count;

    m_mapDummys[ID] = pDummy;
    
    pDummy->Init(channel, zone, m_vecSchedules[scheduleType]);

    pDummy->Start();
    return true;
}

void CDummyManager::AddDummyClient(CClient* pClient)
{
    EnterCriticalSection(&cs);
    m_mapDummyClients[pClient->GetID()] = pClient;
    LeaveCriticalSection(&cs);
}

void CDummyManager::ReleaseDummy(int dummyID)
{
    if (m_mapDummys.find(dummyID) == m_mapDummys.end())
        return;

    const std::vector<CClient*>& vec = m_mapDummys[dummyID]->GetDummyClients();

    EnterCriticalSection(&cs);
    for (int i = 0; i < vec.size(); i++)
    {
        if (m_mapDummyClients.find(vec[i]->GetID()) == m_mapDummyClients.end())
            continue;

        m_mapDummyClients.erase(vec[i]->GetID());
    }
    LeaveCriticalSection(&cs);

    m_mapDummys[dummyID]->Stop();

    m_mapDummys.erase(dummyID);
}

