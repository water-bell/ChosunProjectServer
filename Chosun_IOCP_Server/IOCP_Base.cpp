#include "stdafx.h"
#include "IOCP_Base.h"
#include <process.h>
#include <sstream>
#include <algorithm>
#include <string>

IOCPBase::IOCPBase()
{
	bWorkerThread = TRUE;
	bAccept = TRUE;
}

IOCPBase::~IOCPBase()
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
}

bool IOCPBase::Initialize()
{
	WSADATA wsa = { 0 };
	//winsock 2.2 �������� �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
	{
		puts("ERROR: ������ �ʱ�ȭ �� �� �����ϴ�.");
		return 0;
	}
	//���� ��� ���� ����
	ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ListenSocket == INVALID_SOCKET)
	{
		puts("ERROR: ���� ��� ������ ������ �� �����ϴ�.");
		return 0;
	}
	//���� ���ε�
	SOCKADDR_IN svaddr = { 0 };
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(SERVER_PORT); 
	svaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (::bind(ListenSocket, (SOCKADDR*)&svaddr, sizeof(svaddr)) == SOCKET_ERROR)
	{
		printf_s("ERROR: ���Ͽ� IP�ּҿ� ��Ʈ#�� ���ε��� �� �����ϴ�.");
		return 0;
	}

	//���� ��� ���·� ��ȯ
	if (::listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		printf_s("ERROR: ���� ���·� ��ȯ�� �� �����ϴ�.");
		return 0;
	}

	return TRUE;
}

void IOCPBase::StartServer()
{
	SOCKADDR_IN clientaddr = { 0 };
	int nAddrLen = sizeof(clientaddr);
	SOCKET clientSocket = 0;
	DWORD recvBytes;
	DWORD flags = 0;
	int nReceive = 0;

	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (!CreateWorkerThread()) return;

	puts("������ ���۵Ǿ����ϴ�");
	
	while (bAccept)
	{
		clientSocket = ::accept(ListenSocket, (SOCKADDR*)&clientaddr, &nAddrLen);

		if (clientSocket == INVALID_SOCKET)
		{
			printf_s("ERROR: Ŭ���̾�Ʈ ���� ����\n");
			return;
		}
		else
		{
			printf_s("[Info] �� Ŭ���̾�Ʈ�� ����Ǿ����ϴ�\n");
		}


		SocketInfo = new stSOCKETINFO();
		SocketInfo->socket = clientSocket;
		SocketInfo->recvBytes = 0;
		SocketInfo->sendBytes = 0;
		SocketInfo->dataBuf.len = MAX_BUFFER;
		SocketInfo->dataBuf.buf = SocketInfo->messageBuffer;

		hIOCP = CreateIoCompletionPort((HANDLE)clientSocket, hIOCP, (DWORD)SocketInfo, 0);

		nReceive = WSARecv(
			SocketInfo->socket,
			&SocketInfo->dataBuf,
			1,
			&recvBytes,
			&flags,
			&(SocketInfo->overlapped),
			NULL
		);

		if(nReceive == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			printf_s("ERROR: IO Pending ����: %d", WSAGetLastError());
			return;
		}

	}
}

bool IOCPBase::CreateWorkerThread()
{
	return FALSE;
}

void IOCPBase::Send(stSOCKETINFO* pSocket)
{
	int nResult;
	DWORD sendBytes;
	DWORD dwFlags = 0;

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
		printf_s("Error WSASend ���� : %d", WSAGetLastError());
	}

}

void IOCPBase::Recv(stSOCKETINFO* pSocket)
{
	int nResult;
	DWORD dwFlags = 0;

	ZeroMemory(&(pSocket->overlapped), sizeof(OVERLAPPED));
	ZeroMemory(pSocket->messageBuffer, MAX_BUFFER);
	pSocket->dataBuf.len = MAX_BUFFER;
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->recvBytes = 0;
	pSocket->sendBytes = 0;

	nResult = WSARecv(
		pSocket->socket,
		&(pSocket->dataBuf),
		1,
		(LPDWORD) &pSocket,
		&dwFlags,
		(LPWSAOVERLAPPED) &(pSocket->overlapped),
		NULL
	);

	if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		printf_s("Error WSARecv ���� : %d", WSAGetLastError());
	}

}

void IOCPBase::WorkerThread()
{

}
