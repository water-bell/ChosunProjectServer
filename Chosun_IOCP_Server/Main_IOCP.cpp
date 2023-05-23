#include "stdafx.h"
#include "Main_IOCP.h"
#include <process.h>
#include <sstream>
#include <algorithm>
#include <string>

// static 변수 초기화
int					MainIOCP::Damage = 0;
map<int, SOCKET>	MainIOCP::SessionSocket;
cCharactersInfo		MainIOCP::CharactersInfo;
DBConnector			MainIOCP::Conn;
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

	if (Conn.Connect(DB_ADDRESS, DB_ID, DB_PW, DB_SCHEMA, DB_PORT))
	{
		printf_s("[INFO] DB 접속 성공\n");
	}
	else
	{
		printf_s("[ERROR] DB 접속 실패\n");
	}

	//패킷 함수 포인터에 함수 할당
	fnProcess[EPacketType::SIGNUP].funcProcessPacket = SignUp;
	fnProcess[EPacketType::LOGIN].funcProcessPacket = Login;
	//CommonClass.h에서 EPacketType 수정 필요

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

	Conn.Close();
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
	printf_s("[INFO] CPU 갯수: %d\n", sysInfo.dwNumberOfProcessors);

	nThreadCnt = sysInfo.dwNumberOfProcessors * 2;

	hWorkerHandle = new HANDLE[nThreadCnt];

	for (int i = 0; i < nThreadCnt; i++)
	{
		hWorkerHandle[i] = (HANDLE*)_beginthreadex(
			NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &threadId
		);
		if (hWorkerHandle[i] == NULL)
		{
			printf_s("[INFO] Worker Thread 생성 실패\n");
			return FALSE;
		}
		ResumeThread(hWorkerHandle[i]);
	}
	printf_s("[INFO] Worker Thread 시작 \n");
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
		printf_s("[ERROR] WSASend 실패 : %d\n", WSAGetLastError());
	}
}

void MainIOCP::WorkerThread()
{
	// 함수 호출 여부
	BOOL	bResult;	
	int		nResult;
	// Overlapped I/O 작업에서 전송된 데이터 크기
	DWORD	recvBytes;
	DWORD	sendBytes;
	// Completion Key를 받을 포인터 변수
	stSOCKETINFO* pCompletionKey;
	// I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터
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
			printf_s("[INFO] socket %d 접속 끊어짐\n", pSocketInfo->socket);
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
				printf_s("[ERROR] 정의 되지 않은 패킷 : %d\n", PacketType);
			}
		}
		catch (const std::exception &e)
		{
			printf_s("[ERROR] 알 수 없는 예외 발생 : %s\n", e.what());
		}

		Recv(pSocketInfo);
	}
}

void MainIOCP::SignUp(stringstream& RecvStream, stSOCKETINFO* pSocket)
{
	string Id;
	string Pw;

	RecvStream >> Id;
	RecvStream >> Pw;
	
	printf_s("[INFO] 회원 가입 시도 %s / %s\n", Id, Pw);

	stringstream SendStream;
	SendStream << EPacketType::SIGNUP << endl;
	SendStream << Conn.SignUpAccount(Id, Pw) << endl;

	CopyMemory(pSocket->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->dataBuf.len = SendStream.str().length();

	Send(pSocket);
}

void MainIOCP::Login(stringstream& RecvStream, stSOCKETINFO* pSocket)
{
	string Id;
	string Pw;

	RecvStream >> Id;
	RecvStream >> Pw;

	printf_s("[INFO] 로그인 시도 %s / %s\n", Id, Pw);

	stringstream SendStream;
	SendStream << EPacketType::LOGIN << endl;
	SendStream << Conn.SearchAccount(Id, Pw) << endl;

	CopyMemory(pSocket->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->dataBuf.len = SendStream.str().length();

	Send(pSocket);
}

void MainIOCP::LogoutCharacter(stringstream& RecvStream, stSOCKETINFO* pSocket)
{
	int SessionId;
	RecvStream >> SessionId;
	
	printf_s("[INFO] '%d' 로그아웃 요청\n", SessionId);
	SessionSocket.erase(SessionId);
}

void MainIOCP::Broadcast(stringstream& SendStream)
{
	stSOCKETINFO* client = new stSOCKETINFO;

	for (const auto& kvp : SessionSocket)
	{
		client->socket = kvp.second;
		CopyMemory(client->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
		client->dataBuf.buf = client->messageBuffer;
		client->dataBuf.len = SendStream.str().length();

		Send(client);
	}
}