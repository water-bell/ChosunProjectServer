#include "stdafx.h"
#include "Main_IOCP.h"
#include <process.h>
#include <sstream>
#include <algorithm>
#include <string>

// static ���� �ʱ�ȭ
int					MainIOCP::Damage = 0;
map<int, SOCKET>	MainIOCP::SessionSocket;
cCharactersInfo		MainIOCP::CharactersInfo;
//DBConnector		MainIOCP::Conn;
CRITICAL_SECTION	MainIOCP::csPlayers;

unsigned int WINAPI CallWorkerThread(LPVOID p)
{
	MainIOCP* pOverlappedEvent = (MainIOCP*)p;
	pOverlappedEvent->WorkerThread();
	return 0;
}

MainIOCP::MainIOCP()
{
	InitializeCriticalSection(&csPlayers);

	//DB���� �ڵ� �ۼ�


	//��Ŷ �Լ� �����Ϳ� �Լ� �Ҵ�
	fnProcess[EPacketType::SIGNUP].funcProcessPacket = SignUp;
	fnProcess[EPacketType::LOGIN].funcProcessPacket = Login;
	//CommonClass.h���� EPacketType ���� �ʿ�

}

MainIOCP::~MainIOCP()
{
	WSACleanup();
	if (SocketInfo)
	{
		delete[] SocketInfo;
		SocketInfo = NULL;
	}

	if (hWorkerHandle)
	{
		delete[] hWorkerHandle;
		hWorkerHandle = NULL;
	}

	//Conn.Close();
}

void MainIOCP::StartServer()
{
	IOCPBase::StartServer();
}

bool MainIOCP::CreateWorkerThread()
{
	unsigned int	threadId;
	
	SYSTEM_INFO		sysInfo;
	GetSystemInfo(&sysInfo);
	printf_s("[INFO] CPU ����: %d\n", sysInfo.dwNumberOfProcessors);

	nThreadCnt = sysInfo.dwNumberOfProcessors * 2;

	hWorkerHandle = new HANDLE[nThreadCnt];

	for (int i = 0; i < nThreadCnt; i++)
	{
		hWorkerHandle[i] = (HANDLE*)_beginthreadex(
			NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &threadId
		);
		if (hWorkerHandle[i] == NULL)
		{
			printf_s("[INFO] Worker Thread ���� ����\n");
			return FALSE;
		}
		ResumeThread(hWorkerHandle[i]);
	}
	printf_s("[INFO] Worker Thread ���� \n");
	return TRUE;
}

void MainIOCP::Send(stSOCKETINFO* pSocket)
{
	int		nResult;
	DWORD	sendBytes;
	DWORD	dwFlags = 0;

	nResult = WSASend(
		pSocket->socket,
		&(pSocket->dataBuf),
		1,
		&sendBytes,
		dwFlags,
		NULL,
		NULL
	);

	if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		printf_s("[ERROR] WSASend ���� : %d\n", WSAGetLastError());
	}
}

void MainIOCP::WorkerThread()
{
	// �Լ� ȣ�� ����
	BOOL	bResult;	
	int		nResult;
	// Overlapped I/O �۾����� ���۵� ������ ũ��
	DWORD	recvBytes;
	DWORD	sendBytes;
	// Completion Key�� ���� ������ ����
	stSOCKETINFO* pCompletionKey;
	// I/O �۾��� ���� ��û�� Overlapped ����ü�� ���� ������
	stSOCKETINFO* pSocketInfo;
	DWORD	dwFlags = 0;

	while (bWorkerThread)
	{
		bResult = GetQueuedCompletionStatus(
			hIOCP,
			&recvBytes,
			(PULONG_PTR)&pCompletionKey,
			(LPOVERLAPPED*)&pSocketInfo,
			INFINITE
		);

		if (!bResult && recvBytes == 0)
		{
			printf_s("[INFO] socket #%d ���� ������\n", pSocketInfo->socket);
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
			continue;
		}

		pSocketInfo->dataBuf.len = recvBytes;

		if (recvBytes == 0)
		{
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
			continue;
		}

		try
		{
			int PacketType;
			stringstream RecvStream;

			RecvStream << pSocketInfo->dataBuf.buf;
			RecvStream >> PacketType;

			if (fnProcess[PacketType].funcProcessPacket != nullptr)
			{
				fnProcess[PacketType].funcProcessPacket(RecvStream, pSocketInfo);
			}
			else
			{
				printf_s("[ERROR] ���� ���� ���� ��Ŷ : %d\n", PacketType);
			}
		}
		catch (const std::exception &e)
		{
			printf_s("[ERROR] �� �� ���� ���� �߻� : %s\n", e.what());
		}

		Recv(pSocketInfo);
	}
}