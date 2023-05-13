#pragma once

// 멀티바이트 집합 사용시 define
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// winsock2 사용을 위해 아래 코멘트 추가
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <map>
#include <vector>
#include <iostream>

using namespace std;

#define	MAX_BUFFER		4096
#define SERVER_PORT		8000
#define MAX_CLIENTS		100

//IOCP 소켓 구조체
struct stSOCKETINFO
{
	WSAOVERLAPPED overlapped;
	WSABUF		  dataBuf;
	SOCKET		  socket;
	char		  messageBuffer[MAX_BUFFER];
	int			  recvBytes;
	int			  sendBytes;
};

//패킷 처리를 위한 함수 포인터
struct FuncProcess
{
	void(*funcProcessPacket)(stringstream& RecvStream, stSOCKETINFO* pSocket);
	FuncProcess()
	{
		funcProcessPacket = nullptr;
	}
};

class IOCPBase
{
public:
	IOCPBase();
	virtual ~IOCPBase();

	bool Initialize();
	virtual void StartServer();
	virtual bool CreateWorkerThread();
	virtual void WorkerThread();
	virtual void Send(stSOCKETINFO* pSocket);
	virtual void Recv(stSOCKETINFO* pSocket);

protected:
	stSOCKETINFO* SocketInfo;
	SOCKET		  ListenSocket;
	HANDLE		  hIOCP;
	bool		  bAccept;
	bool          bWorkerThread;
	HANDLE*		  hWorkerHandle;
	int			  nThreadCnt;
};