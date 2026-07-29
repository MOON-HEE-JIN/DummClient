#pragma once
#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include "../CUtill/CRingBuffer.h"
#include "../CUtill/CPacket.h"
#include <queue>
#include <vector>
#include <map>

#include "../Log/LogDefines.h"

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
	CRITICAL_SECTION m_csSendTime;

	BOOL bConnect;
	bool m_bWsaStarted;
private:
	HANDLE CICP;
protected:
	LARGE_INTEGER freq;
	
	std::vector<int> m_vecEnqueueType;
	std::map<int, std::queue<LONGLONG>> m_mapSendTime;

	
	void PushSendTime(int type, LONGLONG time);
	LONGLONG PopSendTime(int type);
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
public:
	virtual void OnRecv(int type, CPacket& cPacket, LONGLONG recvtime = 0) = 0;
	int Connect(const char IP[16], unsigned short Port, HANDLE cicp);
	void Clear();
	void CloseSocket();
	int SendPacket(CPacket* _pPacket);
	void SendEnqueuePacket(int type, CPacket* _pPacket);
	int SendPost();
	void RecvPost();
};
