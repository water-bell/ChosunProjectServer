#pragma once

// ��Ƽ����Ʈ ���� ���� define
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// winsock2 ����� ���� �Ʒ� �ڸ�Ʈ �߰�
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <map>
#include <vector>
#include <iostream>

using namespace std;

#define	MAX_BUFFER		4096
#define SERVER_PORT		8000
#define MAX_CLIENTS		100

//IOCP ���� ����ü
struct stSOCKETINFO
{
	WSAOVERLAPPED overlapped;
	WSABUF		  dataBuf;
	SOCKET		  socket;
	char		  messageBuffer[MAX_BUFFER];
	int			  recvBytes;
	int			  sendBytes;
};

//��Ŷ ó���� ���� �Լ� ������
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