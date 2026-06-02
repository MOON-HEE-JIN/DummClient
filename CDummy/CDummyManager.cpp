#include "CDummyManager.h"
#include "../Test/TSchedule_Change_Zone.h"
#include "../Test/TSchedule_Move.h"
#include "../Test/TSchedule_LoopBack.h"
#include "../Test/TSchedule_PlaceMainworld.h"
#include "../Log/CLog.h"

CDummyManager g_DummyManager;

CDummyManager::CDummyManager()
{
    m_iDummyID = 0;
    m_iClientID = 0;

    memcpy(m_szIP, "127.0.0.1", 10);
    m_sPort = 7799;

    m_vecSchedules.push_back(new TSchedule_Change_Zone());
    m_vecSchedules.push_back(new TSchedule_Move());
    m_vecSchedules.push_back(new TSchedule_LoopBack());
    CSchedule* pSchedule = new TSchedule_PlaceMainWorld();
    ((TSchedule_PlaceMainWorld*)pSchedule)->Init(1024, 1024, 4, 4);
    m_vecSchedules.push_back(pSchedule);

    InitializeCriticalSection(&cs);

    m_vecDummyThreadHandles.resize(5);
    m_vecThreadDummyClientCount.resize(5);
    m_vecThreadLock.resize(5);

    m_hExit = CreateEvent(NULL, TRUE, FALSE, NULL);
  
    for (int i = 0; i < 5; i++)
    {
        m_vecDummyThreadHandles[i] = (HANDLE)_beginthreadex(NULL, 0, RunThread, this, 0, NULL);
        m_vecThreadDummyClientCount[i] = 0;
        m_vecThreadLock[i] = new st_ThreadLock();
    }
}

CDummyManager::~CDummyManager()
{
    m_bRun = false;
    
    int waitthreadcount = m_vecDummyThreadHandles.size();
    for (int i = 0; i < waitthreadcount; i++)
        SetEvent(m_hExit);

    WaitForMultipleObjects(waitthreadcount, m_vecDummyThreadHandles.data(), true, INFINITE);

    int Loop = m_vecThreadLock.size();
    for (int i = 0; i < Loop; i++)
    {
        delete m_vecThreadLock[i];
    }
}

bool CDummyManager::CreateDummy(int channel, int zone, int count, int scheduleType)
{
    int ID = m_iDummyID++;
    CDummy* pDummy = new CDummy(ID, m_szIP, m_sPort, count, m_iClientID);
    m_iClientID += count;

    m_mapDummys[ID] = pDummy;

    pDummy->Init(channel, zone, m_vecSchedules[scheduleType]);
    RegisterThread(pDummy);
  
    return true;
}

void CDummyManager::AddDummyClient(CClient* pClient)
{
    EnterCriticalSection(&cs);
    m_mapDummyClients[pClient->GetID()] = pClient;
    LeaveCriticalSection(&cs);
}


unsigned __stdcall CDummyManager::RunThread(void* arg)
{
    CDummyManager* p = (CDummyManager*)arg;
    static int staticID = 0;

    int ThreadID = staticID++;

    p->Run(ThreadID);

    return 0;
}

void CDummyManager::Run(const int id)
{
    int ret;
    std::vector<CDummy*> vec;
    while (m_bRun)
    {
        ret = WaitForSingleObject(m_hExit, 1);

        if (ret == WAIT_OBJECT_0)
            break;

        if (m_vecThreadLock[id]->bChange.load())
        {
            m_vecThreadLock[id]->bChange.store(false);
            m_vecThreadLock[id]->Lock();
            vec = m_mapThreadDummy[id];    // 복사해서 가져오기
            m_vecThreadLock[id]->UnLock();
        }
        
        int Loop = vec.size();
        double maxTime = 0, minTime = 9999999, avgTime = 0;
        int maxComplete = 0, minComplete = 999999999;
        
        for (int i = 0; i < Loop; i++)
        {
            vec[i]->Update();
            maxTime = max(maxTime, vec[i]->GetMaxTime());
            minTime = min(minTime, vec[i]->GetMinTime());
            avgTime += vec[i]->GetAvgTime();
            maxComplete = max(maxComplete, vec[i]->GetMaxComplete());
            minComplete = min(minComplete, vec[i]->GetMinComplete());
        }
        avgTime /= Loop;

        if (Loop > 0)
        {
            if (m_iLatencyTime + m_iDelayLatencyTime > GetTickCount())
                continue;

            g_LogDummy.ILog("MaxTime : %.3f, MinTime : %.3f, AvgTime : %.3f, MaxComplete : %d, MinComplete : %d",
                maxTime, minTime, avgTime, maxComplete, minComplete);
        
            m_iLatencyTime = GetTickCount();
        }
    }
}

void CDummyManager::RegisterThread(CDummy* pDummy)
{
    // 개수 체크 부터
    bool bNew = true;
    
    int clientcount = pDummy->GetDummyClients().size();
    int Loop = m_vecThreadDummyClientCount.size();
    int registerId;
    for (int i = 0; i < Loop; i++)
    {
        if (m_vecThreadDummyClientCount[i] + clientcount < THREAD_CLIENT_COUNT)
        {
            registerId = i;
            bNew = false;
            break;
        }
    }

    if (bNew)
    {
        m_vecDummyThreadHandles.push_back((HANDLE)_beginthreadex(NULL, 0, RunThread, this, 0, NULL));
        m_vecThreadDummyClientCount.push_back(0);
        m_mapThreadDummy[Loop].push_back(pDummy);
        m_vecThreadLock.push_back(new st_ThreadLock());
        m_vecThreadLock.back()->bChange.store(true);
    }
    else
    {
        m_vecThreadDummyClientCount[registerId] += clientcount;
        m_vecThreadLock[registerId]->Lock();
        m_mapThreadDummy[registerId].push_back(pDummy);
        m_vecThreadLock[registerId]->UnLock();
        m_vecThreadLock[registerId]->bChange.store(true);
    }
}

