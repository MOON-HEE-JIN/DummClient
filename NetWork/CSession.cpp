#include "CSession.h"
#include <WS2tcpip.h>
#include "../Stub/StructDef.h"

CSession::CSession()
{
	CICP = 0;
	sock = 0;

	RecvQ = new RingQueue;
	SendQ = new RingQueue;

	RecvOverlap = { 0 };
	SendOverlap = { 0 };

	bSendFlag = false;
	UseFlag = true;

	bConnect = false;

	IOCnt = 0;

	m_DebugTotalNetTime = 0;
	m_DebugTotalCount = 0;

	InitializeCriticalSection(&cs);
	InitializeCriticalSection(&m_csSendQ);
	InitializeCriticalSection(&m_DebugCSTime);
}

CSession::~CSession()
{
	CloseSocket();

	delete RecvQ;
	delete SendQ;
	DeleteCriticalSection(&cs);
	DeleteCriticalSection(&m_csSendQ);
	DeleteCriticalSection(&m_DebugCSTime);
}

BOOL CSession::GetQueueEmpty()
{
	EnterCriticalSection(&m_DebugCSTime);
	bool ret = m_DebugTimeQueue.empty();
	LeaveCriticalSection(&m_DebugCSTime);
	return ret;
}

DWORD CSession::GetSendTime()
{
	EnterCriticalSection(&m_DebugCSTime);
	DWORD ret = m_DebugTimeQueue.front();
	m_DebugTimeQueue.pop();
	LeaveCriticalSection(&m_DebugCSTime);
	return ret;
}

void CSession::AddSRNetTime(DWORD t)
{
	m_DebugTotalNetTime += t;
	m_DebugTotalCount.fetch_add(1);
	m_DebugAvgTime.store((float)m_DebugTotalNetTime / m_DebugTotalCount.load());
}

int CSession::Connect(const char IP[16], unsigned short Port, HANDLE cicp)
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return WSAGetLastError();

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		return WSAGetLastError();

	SOCKADDR_IN addr_in;
	ZeroMemory(&addr_in, sizeof(SOCKADDR_IN));
	addr_in.sin_family = AF_INET;

	inet_pton(AF_INET, IP, &addr_in.sin_addr);

	addr_in.sin_port = htons(Port);

	if (connect(sock, (SOCKADDR*)&addr_in, sizeof(addr_in)) == SOCKET_ERROR)
		return WSAGetLastError();
	
	u_long on = 1;
	if (ioctlsocket(sock, FIONBIO, &on) == SOCKET_ERROR)
		return WSAGetLastError();

	bConnect = true;
	
	CICP = CreateIoCompletionPort((HANDLE)sock, cicp, (ULONG_PTR)this, 0);

	RecvPost();

	return 0;
}

void CSession::Clear()
{
	bSendFlag = false;
	UseFlag = false;
	RecvOverlap = { 0 };
	SendOverlap = { 0 };
	RecvQ->Clear();
	SendQ->Clear();

	m_DebugTotalNetTime = 0;
	m_DebugTotalCount = 0;

	while (!m_DebugTimeQueue.empty())
	{
		m_DebugTimeQueue.pop();
	}

	CloseSocket();
}

void CSession::CloseSocket()
{
	if (bConnect)
	{
		bConnect = false;
		closesocket(sock);
		Clear();
	}
}

int CSession::SendPacket(CPacket* _packet)
{
	EnterCriticalSection(&m_csSendQ);
	
	int ret = SendQ->Enqueue(_packet->GetReadBuffPtr(), _packet->GetDataSize());
	LeaveCriticalSection(&m_csSendQ);

	return SendPost();
	//printf("SendPacket\n");
}

void CSession::SendEnqueuePacket(CPacket* _pPacket)
{
	EnterCriticalSection(&m_csSendQ);
	
	int ret = SendQ->Enqueue(_pPacket->GetReadBuffPtr(), _pPacket->GetDataSize());
	LeaveCriticalSection(&m_csSendQ);
}

int CSession::SendPost()
{
	int len = 0;
	if (InterlockedExchange(&bSendFlag, TRUE) == TRUE)
		return 0;

	if (SendQ->GetUseSize() <= 0)
	{
		InterlockedExchange(&bSendFlag, FALSE);
		return 0;
	}
	InterlockedIncrement(&IOCnt);

	int ret;

	EnterCriticalSection(&m_csSendQ);
	if (SendQ->GetDirectDequeueSize() < SendQ->GetUseSize())
	{
		WSABUF wsabuf[2];
		wsabuf[0].buf = SendQ->GetReadPointer();
		wsabuf[0].len = SendQ->GetDirectDequeueSize();
		wsabuf[1].buf = SendQ->GetFirstPointer();
		wsabuf[1].len = SendQ->GetUseSize() - SendQ->GetDirectDequeueSize();
		len = wsabuf[0].len + wsabuf[1].len;
		ret = WSASend(sock, wsabuf, 2, 0, 0, &SendOverlap, NULL);
	}
	else
	{
		WSABUF wsabuf;
		wsabuf.buf = SendQ->GetReadPointer();
		wsabuf.len = SendQ->GetDirectDequeueSize();
		len = wsabuf.len;
		ret = WSASend(sock, &wsabuf, 1, 0, 0, &SendOverlap, NULL);
	}
	DWORD t = GetTickCount();
	if (ret != SOCKET_ERROR)
	{
		EnterCriticalSection(&m_DebugCSTime);
		m_DebugTimeQueue.push(t);
		LeaveCriticalSection(&m_DebugCSTime);
	}

	LeaveCriticalSection(&m_csSendQ);

	if (ret == SOCKET_ERROR)
	{
		ret = WSAGetLastError();
		if (ret != WSA_IO_PENDING)
		{
			/*
			if (ret != 10038 && ret != 10054 && ret != WSA_IO_PENDING)
				LOG_INFO("SEND_WSA_ERROR_%d\n", ret);
			*/

			InterlockedExchange((DWORD*)&bSendFlag, FALSE);
			if (InterlockedDecrement((DWORD*)&IOCnt) == 0)
			{
				closesocket(sock);
			}
		}
		else
		{
			//printf("%d Send IO_PENDING\n", (int)sock);
		}
	}
	return len;
}

void CSession::RecvPost()
{
	IncrementIOCnt();
	DWORD flags = 0;

	int ret = 0;

	ZeroMemory(&RecvOverlap, sizeof(OVERLAPPED));
	if (RecvQ->GetDirectEnqueueSize() < RecvQ->GetFreeSize())
	{
		WSABUF wsabuf[2];
		wsabuf[0].buf = RecvQ->GetWritePointer();
		wsabuf[0].len = RecvQ->GetDirectEnqueueSize();
		wsabuf[1].buf = RecvQ->GetFirstPointer();
		wsabuf[1].len = RecvQ->GetFreeSize() - wsabuf[0].len;

		ret = WSARecv(sock, wsabuf, 2, NULL, &flags, &RecvOverlap, NULL);
	}

	else
	{
		WSABUF wsabuf;
		wsabuf.buf = RecvQ->GetWritePointer();
		wsabuf.len = RecvQ->GetDirectEnqueueSize();

		ret = WSARecv(sock, &wsabuf, 1, NULL, &flags, &RecvOverlap, NULL);
	}

	if (ret == SOCKET_ERROR)
	{
		ret = WSAGetLastError();
		if (ret != WSA_IO_PENDING)
		{
			// 10054 : 연결이 강제로 끊김, 10053 : 비정상 종료
			if (ret != 10054 && ret != 10053)
			{
				printf("-- Recv WSARecv Error %d ---\n", ret);
			}
			if (InterlockedDecrement(&IOCnt) == 0)
			{
				CloseSocket();
			}
		}
	}
}