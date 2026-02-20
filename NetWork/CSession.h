#pragma once
#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include "../CUtill/RingQueue.h"
#include "../CUtill/CPacket.h"
#include <queue>
#include <atomic>

class CSession
{
public:
	CSession();
	~CSession();

private:
	SOCKET sock;

	RingQueue* SendQ;
	RingQueue* RecvQ;

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
public:
	RingQueue* GetSendBuffer() { return SendQ; }
	RingQueue* GetRecvBuffer() { return RecvQ; }

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
	void SendPacket(int _type, CPacket* _pPacket);
	void SendEnqueuePacket(int _type, CPacket* _pPacket);
	void SendPost();
	void RecvPost();
};
