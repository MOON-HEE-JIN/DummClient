#include "CDummyManager.h"

#include "../Test/TSchedule_Change_Zone.h"
#include "../Test/TSchedule_Move.h"
#include "../Test/TSchedule_LoopBack.h"
#include "../Test/TSchedule_PlaceMainworld.h"
#include "../Test/TSchedule_MainWorldMoveAoi.h"
#include "../Test/TSchedule_MonitorAoiTile.h"
#include "../Test/TSchedule_LoopBack_ChangeZone.h"
#include "../Test/TSchedule_LoopBack_DisReConnect.h"


#include "../Log/CLog.h"
#include <process.h>

CDummyManager g_DummyManager;

CDummyManager::CDummyManager()
{
    m_iDummyID = 0;
    m_iClientID = 0;

    strcpy_s(m_szIP, "127.0.0.1");
    m_sPort = 7799;

    m_vecSchedules.resize(ESCHEDULE_TEST_TYPE::SCHEDULE_END);

    {
        CSchedule* pSchedule = new TSchedule_Change_Zone();
        m_vecSchedules[pSchedule->GetType()] = pSchedule;
    }

    {
        CSchedule* pSchedule = new TSchedule_Move();
        m_vecSchedules[pSchedule->GetType()] = pSchedule;
    }

    {
        CSchedule* pSchedule = new TSchedule_LoopBack();
		m_vecSchedules[pSchedule->GetType()] = pSchedule;
    }

    {
        CSchedule* pSchedule = new TSchedule_PlaceMainWorld();
        ((TSchedule_PlaceMainWorld*)pSchedule)->Init(1024, 1024, 4, 4);
        m_vecSchedules[pSchedule->GetType()] = pSchedule;
    }
    
    {
        CSchedule* pSchedule = new TSchedule_MonitorAoiTile();
		((TSchedule_MonitorAoiTile*)pSchedule)->Init(1024, 1024, 64, 64);
		m_vecSchedules[pSchedule->GetType()] = pSchedule;
    }

    {
        CSchedule* pSchedule = new TSchedule_MainWorldMoveAoi();
        ((TSchedule_MainWorldMoveAoi*)pSchedule)->Init(1024, 1024, 4, 4);
        m_vecSchedules[pSchedule->GetType()] = pSchedule;
    }
    
    {
        CSchedule* pSchedule = new TSchedule_LoopBack_ChangeZone();
        m_vecSchedules[pSchedule->GetType()] = pSchedule;
    }

    {
        CSchedule* pSchedule = new TSchedule_LoopBack_DisReConnect();
        m_vecSchedules[pSchedule->GetType()] = pSchedule;
    }

    InitializeCriticalSection(&cs);

    m_vecDummyThreadHandles.resize(5);
    m_vecThreadDummyClientCount.resize(5);
    m_vecThreadLock.resize(5);

    m_hExit = CreateEvent(NULL, TRUE, FALSE, NULL);
  
    for (int i = 0; i < 5; i++)
    {
        m_vecThreadDummyClientCount[i] = 0;
        m_vecThreadLock[i] = new st_ThreadLock();
    }

    // 작업 자료구조를 모두 만든 뒤 스레드를 시작해 초기 접근 경쟁을 막는다.
    for (int i = 0; i < 5; i++)
        m_vecDummyThreadHandles[i] = (HANDLE)_beginthreadex(NULL, 0, RunThread, this, 0, NULL);
}

CDummyManager::~CDummyManager()
{
    m_bRun.store(false);
    
    SetEvent(m_hExit);

    for (HANDLE handle : m_vecDummyThreadHandles)
    {
        if (handle == NULL)
            continue;
        WaitForSingleObject(handle, INFINITE);
        CloseHandle(handle);
    }

    for (auto& [id, dummy] : m_mapDummys)
        delete dummy;
    m_mapDummys.clear();
    m_mapDummyClients.clear();

    for (st_ThreadLock* lock : m_vecThreadLock)
        delete lock;

    for (CSchedule* schedule : m_vecSchedules)
        delete schedule;

    if (m_hExit != NULL)
        CloseHandle(m_hExit);
    DeleteCriticalSection(&cs);
}

bool CDummyManager::CreateDummy(int channel, int zone, int count, int scheduleType)
{
    CSchedule* schedule = GetSchedule(scheduleType);
    if (count <= 0 || schedule == nullptr)
    {
        g_LogDummy.ELog("ERROR Invalid Dummy Config count[%d] schedule[%d]", count, scheduleType);
        return false;
    }

    if (scheduleType == ESCHEDULE_TEST_TYPE::SCHEDULE_MAIN_WORLD)
    {
        TSchedule_PlaceMainWorld* mainWorldSchedule = dynamic_cast<TSchedule_PlaceMainWorld*>(schedule);
        if (mainWorldSchedule == nullptr || count != mainWorldSchedule->GetTileCount())
        {
            g_LogDummy.ELog(
                "ERROR MainWorld requires one client per tile. client[%d] tile[%d]",
                count,
                mainWorldSchedule == nullptr ? 0 : mainWorldSchedule->GetTileCount());
            return false;
        }

        mainWorldSchedule->PrepareRun(m_iClientID, count);
    }

    if (scheduleType == ESCHEDULE_TEST_TYPE::SCHEDULE_MAIN_WORLD_MOVE_AOI)
    {
        TSchedule_MainWorldMoveAoi* mainWorldSchedule =
            dynamic_cast<TSchedule_MainWorldMoveAoi*>(schedule);
        if (mainWorldSchedule == nullptr || count != mainWorldSchedule->GetTileCount())
        {
            g_LogDummy.ELog(
                "ERROR MainWorld Move/AOI requires one client per tile. client[%d] tile[%d]",
                count,
                mainWorldSchedule == nullptr ? 0 : mainWorldSchedule->GetTileCount());
            return false;
        }
    }

    const int ID = m_iDummyID++;
    CDummy* pDummy = new CDummy(ID, m_szIP, m_sPort, count, m_iClientID);
    m_iClientID += count;

    if (!pDummy->Init(channel, zone, schedule))
    {
        delete pDummy;
        --m_iDummyID;
        m_iClientID -= count;
        return false;
    }

    m_mapDummys[ID] = pDummy;
    RegisterThread(pDummy);
  
    return true;
}

CDummy* CDummyManager::GetDummy(int id)
{
    if (m_mapDummys.find(id) == m_mapDummys.end())
        return nullptr;
    return m_mapDummys[id];
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
    static std::atomic<int> nextThreadID = 0;

    const int ThreadID = nextThreadID.fetch_add(1);

    p->Run(ThreadID);

    return 0;
}

void CDummyManager::Run(const int id)
{
    int ret;
    std::vector<CDummy*> vec;
    while (m_bRun.load())
    {
        ret = WaitForSingleObject(m_hExit, 1);

        if (ret == WAIT_OBJECT_0)
            break;

        if (m_vecThreadLock[id]->bChange.exchange(false))
        {
            m_vecThreadLock[id]->Lock();
            vec = m_mapThreadDummy[id];    // 복사해서 가져오기
            m_vecThreadLock[id]->UnLock();
        }
        
        const int Loop = static_cast<int>(vec.size());
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
        if (Loop > 0)
        {
            avgTime /= static_cast<double>(Loop);

            const ULONGLONG now = GetTickCount64();
            ULONGLONG previousLogTime = m_iLatencyTime.load();
            if (previousLogTime + static_cast<ULONGLONG>(m_iDelayLatencyTime) > now)
                continue;
            if (!m_iLatencyTime.compare_exchange_strong(previousLogTime, now))
                continue;

            g_LogDummy.ILog("T[%d] MaxTime : %.3f, MinTime : %.3f, AvgTime : %.3f, MaxComplete : %d, MinComplete : %d",
                id, maxTime, minTime, avgTime, maxComplete, minComplete);
        }
    }
}

void CDummyManager::RegisterThread(CDummy* pDummy)
{
    // 개수 체크 부터
    bool bNew = true;
    
    const int clientcount = static_cast<int>(pDummy->GetDummyClients().size());
    const int Loop = static_cast<int>(m_vecThreadDummyClientCount.size());
    int registerId;
    for (int i = 0; i < Loop; i++)
    {
        if (m_vecThreadDummyClientCount[i] + clientcount <= THREAD_CLIENT_COUNT)
        {
            registerId = i;
            bNew = false;
            break;
        }
    }

    if (bNew)
    {
        m_vecDummyThreadHandles.push_back(NULL);
        m_vecThreadDummyClientCount.push_back(0);
        m_vecThreadLock.push_back(new st_ThreadLock());
        m_vecThreadDummyClientCount[Loop] = clientcount;
        m_mapThreadDummy[Loop].push_back(pDummy);
        m_vecThreadLock.back()->bChange.store(true);
        m_vecDummyThreadHandles[Loop] = (HANDLE)_beginthreadex(NULL, 0, RunThread, this, 0, NULL);
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

