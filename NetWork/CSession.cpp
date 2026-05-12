#include "CSession.h"
#include <WS2tcpip.h>
#include "../Stub/StructDef.h"

CSession::CSession()
{
	CICP = 0;
	sock = 0;

	RecvQ = new CRingBuffer;
	SendQ = new CRingBuffer;

	RecvOverlap = { 0 };
	SendOverlap = { 0 };

	bSendFlag = false;
	UseFlag = true;

	bConnect = false;

	IOCnt = 0;

	QueryPerformanceFrequency(&freq);
	
	InitializeCriticalSection(&cs);
	InitializeCriticalSection(&m_csSendQ);
	InitializeCriticalSection(&m_csSendTime);
}

CSession::~CSession()
{
	CloseSocket();

	delete RecvQ;
	delete SendQ;
	DeleteCriticalSection(&cs);
	DeleteCriticalSection(&m_csSendQ);
	DeleteCriticalSection(&m_csSendTime);
}

void CSession::PushSendTime(int type, double time)
{
	EnterCriticalSection(&m_csSendTime);
	m_mapSendTime[type].push(time);
	LeaveCriticalSection(&m_csSendTime);
}

double CSession::PopSendTime(int type)
{
	double ret = -1;
	EnterCriticalSection(&m_csSendTime);

	if (m_mapSendTime.find(type) == m_mapSendTime.end())
	{
		ret = -1;
	}
	else
	{
		if (m_mapSendTime[type].empty())
		{
			LeaveCriticalSection(&m_csSendTime);
			return -1;
		}
		ret = m_mapSendTime[type].front();
		m_mapSendTime[type].pop();
	}
	LeaveCriticalSection(&m_csSendTime);
	return ret;
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

void CSession::SendEnqueuePacket(int type, CPacket* _pPacket)
{
	EnterCriticalSection(&m_csSendQ);
	m_vecEnqueueType.push_back(type);
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
		wsabuf[0].buf = (char*)SendQ->GetReadPointer();
		wsabuf[0].len = SendQ->GetDirectDequeueSize();
		wsabuf[1].buf = (char*)SendQ->GetBuffer();
		wsabuf[1].len = SendQ->GetUseSize() - SendQ->GetDirectDequeueSize();
		len = wsabuf[0].len + wsabuf[1].len;
		
		LARGE_INTEGER sendtime;
		QueryPerformanceCounter(&sendtime);

		for (int i = 0; i < m_vecEnqueueType.size(); i++)
		{
			PushSendTime(m_vecEnqueueType[i], sendtime.QuadPart);
		}
		m_vecEnqueueType.clear();
		
		ret = WSASend(sock, wsabuf, 2, 0, 0, &SendOverlap, NULL);
	
	}
	else
	{
		WSABUF wsabuf;
		wsabuf.buf = (char*)SendQ->GetReadPointer();
		wsabuf.len = SendQ->GetDirectDequeueSize();
		len = wsabuf.len;

		LARGE_INTEGER sendtime;
		QueryPerformanceCounter(&sendtime);

		// 해당 부분에서 sendtime 에 enqueue 하는 이유는
		//  WSASend 후 바로 Recv 가 올 경우 enqueue 전에 pop 을 하고 시간 측정이 제대로 작동하지 않는다
		// enqueue 시간을 고려하고 시간 확인하기 
		for (int i = 0; i < m_vecEnqueueType.size(); i++)
		{
			PushSendTime(m_vecEnqueueType[i], sendtime.QuadPart);
		}
		m_vecEnqueueType.clear();
		
		ret = WSASend(sock, &wsabuf, 1, 0, 0, &SendOverlap, NULL);
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
		wsabuf[0].buf = (char*)RecvQ->GetWritePointer();
		wsabuf[0].len = RecvQ->GetDirectEnqueueSize();
		wsabuf[1].buf = (char*)RecvQ->GetBuffer();
		wsabuf[1].len = RecvQ->GetFreeSize() - wsabuf[0].len;

		ret = WSARecv(sock, wsabuf, 2, NULL, &flags, &RecvOverlap, NULL);
	}

	else
	{
		WSABUF wsabuf;
		wsabuf.buf = (char*)RecvQ->GetWritePointer();
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