#include "CNetWork.h"
#include "CSession.h"

#include "../Stub/StructDef.h"
#include <Windows.h>
#include <process.h>

bool g_bRun = true;
HANDLE CICP;

static HANDLE hWorkerThread;

void StartThread()
{
	CICP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 3);
	if (CICP == NULL)
		return;

	hWorkerThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, NULL, 0, NULL);
}

void WaitThread()
{
	WaitForSingleObject(hWorkerThread, INFINITE);
}

unsigned __stdcall WorkerThread(void* arg)
{
	int retval = 0;
	CSession* pSession = nullptr;
	DWORD transfrerred;

	ULONG_PTR key;
	while (g_bRun)
	{
		pSession = nullptr;
		transfrerred = 0;
		OVERLAPPED* overlapped = nullptr;
		retval = GetQueuedCompletionStatus(CICP, &transfrerred, &key, &overlapped, INFINITE);
		
		pSession = (CSession*)key;

		if (pSession == nullptr)
			continue;

		if (overlapped == nullptr)
			continue;

		if (overlapped == pSession->GetRecvOverlapPointer())
		{
			if (retval == 0 || transfrerred == 0)
			{
				int err = WSAGetLastError();
				if (err != 64 && err != 997 && err != 0 && err != 10038 && err != 1236)
				{
					/*
					* ERROR_NETNAME_DELETED(64) : TCP 연결이 비정상적 종료
					* WSA_IO_PENDING(997) : 중첩 I/O 작업 나중에 완료
					* ERROR_NETWORK_UNREACHABLE(1236) : 네트워크 연결이 시스템에 의해 중단
					*	linger 옵션이 설정시 RST 를 즉시 전송 RST 에의 해 연결이 종료 되어 대기중인 recv 에서 오류
					* WSAENOTSOCKET(10038) : nonsocket 에 대한 소켓 작업
					printf("WorkerThread GQCS Error %d\n", err);
					*/
				}
				pSession->CloseSocket();
			}
			else
			{
				pSession->GetRecvBuffer()->MoveWritePointer(transfrerred);

				st_Header header;

				int size;

				while (1)
				{
					size = pSession->GetRecvBuffer()->GetUseSize();

					//고정된 크기의 Header 크기 확인
					if (size < sizeof(st_Header))
						break;

					pSession->GetRecvBuffer()->Peek((char*)&header, sizeof(st_Header));

					if (size - sizeof(st_Header) < header.size)
						break;

					pSession->GetRecvBuffer()->MoveReadPointer(sizeof(st_Header));
					CPacket cPacket;

					pSession->GetRecvBuffer()->Dequeue(cPacket.GetWriteBuffPtr(), header.size);
					cPacket.MoveWritePos(header.size);

					pSession->OnRecv(header.type, cPacket);
				}
				pSession->RecvPost();
			}
			pSession->DecrementIOCnt();
		}
		else if (overlapped == pSession->GetSendOverlapPointer())
		{
			pSession->LockSendQ();
			{
				pSession->GetSendBuffer()->MoveReadPointer(transfrerred);
			}
			pSession->UnLockSendQ();

			pSession->ChangeSendFlag(FALSE);

			if (retval == 0 || transfrerred == 0)
			{
				int err = WSAGetLastError();
				if (err != 64 && err != 997 && err != 0 && err != 10038 && err != 1236)
				{
					/*
					* ERROR_NETNAME_DELETED(64) : TCP 연결이 비정상적 종료
					* WSA_IO_PENDING(997) : 중첩 I/O 작업 나중에 완료
					* ERROR_NETWORK_UNREACHABLE(1236) : 네트워크 연결이 시스템에 의해 중단
					*	linger 옵션이 설정시 RST 를 즉시 전송 RST 에의 해 연결이 종료 되어 대기중인 recv 에서 오류
					* WSAENOTSOCKET(10038) : nonsocket 에 대한 소켓 작업
					printf("WorkerThread GQCS Error %d\n", err);
					*/
				}
				pSession->CloseSocket();
			}
			else
			{
				pSession->SendPost();
			}
			pSession->DecrementIOCnt();
		}

		if (pSession->GetIOCnt() == 0)
		{
			delete pSession;
		}
	}
	return 0;
}

void* GetCICPPort()
{
	return CICP;
}
