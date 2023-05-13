#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <map>
#include <vector>
#include <iostream>
//#include "DBConnector.h"
#include "IOCP_Base.h"
#include "CommonClass.h"

using namespace std;

class MainIOCP : public IOCPBase
{
public:
	MainIOCP();
	virtual ~MainIOCP();

	virtual void StartServer() override;
	virtual bool CreateWorkerThread() override;
	virtual void WorkerThread() override;
	static void Send(stSOCKETINFO* pSocket);

private:
	static cCharactersInfo  CharactersInfo;
	static map<int, SOCKET> SessionSocket;
	static int				Damage;
	//static DBConnector Conn; 
	static CRITICAL_SECTION csPlayers;

	FuncProcess				fnProcess[160];
	
	static void SignUp(stringstream& RecvStream, stSOCKETINFO* pSocket);				//ȸ������
	static void Login(stringstream& RecvStream, stSOCKETINFO* pSocket);					// �α���
	static void EnrollCharacter(stringstream& RecvStream, stSOCKETINFO* pSocket);		// ĳ���� ���
	static void SyncCharacters(stringstream& RecvStream, stSOCKETINFO* pSocket);		// ĳ���� ��ġ ����ȭ
	static void LogoutCharacter(stringstream& RecvStream, stSOCKETINFO* pSocket);		// ĳ���� �α׾ƿ� 
	static void HitCharacter(stringstream& RecvStream, stSOCKETINFO* pSocket);			// ĳ���� �ǰ� ó��

	// ��ε�ĳ��Ʈ �Լ�
	static void Broadcast(stringstream& SendStream);
	// �ٸ� Ŭ���̾�Ʈ�鿡�� �� �÷��̾� ���� ���� ����
	static void BroadcastNewPlayer(cCharacter& player);
	// ĳ���� ������ ���ۿ� ���
	static void WriteCharactersInfoToSocket(stSOCKETINFO* pSocket);
};