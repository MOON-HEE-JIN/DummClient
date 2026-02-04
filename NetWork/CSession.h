#pragma once
#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include "../CUtill/RingQueue.h"
#include "../CUtill/CPacket.h"

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

	BOOL bConnect;
private:
	HANDLE CICP;

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
