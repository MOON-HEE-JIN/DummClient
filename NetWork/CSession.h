#pragma once
#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include "../CUtill/CRingBuffer.h"
#include "../CUtill/CPacket.h"
#include <queue>
#include <atomic>
#include <vector>

struct st_DebugHeader
{
	int type;
	int size;
};
class CSession
{
public:
	CSession();
	~CSession();
private:
	SOCKET sock;

	CRingBuffer* SendQ;
	CRingBuffer* RecvQ;

	DWORD IOCnt;
	DWORD bSendFlag;
	DWORD UseFlag;
	OVERLAPPED SendOverlap;
	OVERLAPPED RecvOverlap;

	CRITICAL_SECTION cs;
	CRITICAL_SECTION m_csSendQ;
	CRITICAL_SECTION m_DebugCSTime;
	BOOL bConnect;
private:
	HANDLE CICP;

	std::queue<DWORD> m_DebugTimeQueue;

	LONGLONG m_DebugTotalNetTime;
	std::atomic<int> m_DebugTotalCount;
	std::atomic<float> m_DebugAvgTime;
public:
	int IncrementIOCnt() { return InterlockedIncrement(&IOCnt); }
	int DecrementIOCnt() { return InterlockedDecrement(&IOCnt); }
	int GetIOCnt() { DWORD ret; InterlockedExchange(&ret, IOCnt); return ret; }
	void ChangeSendFlag(bool b) { InterlockedExchange((DWORD*)& bSendFlag, b); }

	void LockSendQ() { EnterCriticalSection(&m_csSendQ); }
	void UnLockSendQ() { LeaveCriticalSection(&m_csSendQ); }

	void LockSession() { EnterCriticalSection(&cs); }
	void UnLockSession() { LeaveCriticalSection(&cs); }
public:
	CRingBuffer* GetSendBuffer() { return SendQ; }
	CRingBuffer* GetRecvBuffer() { return RecvQ; }

	OVERLAPPED* GetSendOverlapPointer() { return &SendOverlap; }
	OVERLAPPED* GetRecvOverlapPointer() { return &RecvOverlap; }

	BOOL		GetConnect() { return bConnect; }
	BOOL		GetQueueEmpty();

	DWORD		GetSendTime();
	float		GetAvgNetTime() { return m_DebugAvgTime.load(); }
	int			GetTotalNetCount() { return m_DebugTotalCount.load(); }

	void		AddSRNetTime(DWORD t);
public:
	virtual void OnRecv(int type, CPacket& cPacket) = 0;
	int Connect(const char IP[16], unsigned short Port, HANDLE cicp);
	void Clear();
	void CloseSocket();
	int SendPacket(CPacket* _pPacket);
	void SendEnqueuePacket(CPacket* _pPacket);
	int SendPost();
	void RecvPost();
};
