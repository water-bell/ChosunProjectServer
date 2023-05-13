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
	
	static void SignUp(stringstream& RecvStream, stSOCKETINFO* pSocket);				//회원가입
	static void Login(stringstream& RecvStream, stSOCKETINFO* pSocket);					// 로그인
	static void EnrollCharacter(stringstream& RecvStream, stSOCKETINFO* pSocket);		// 캐릭터 등록
	static void SyncCharacters(stringstream& RecvStream, stSOCKETINFO* pSocket);		// 캐릭터 위치 동기화
	static void LogoutCharacter(stringstream& RecvStream, stSOCKETINFO* pSocket);		// 캐릭터 로그아웃 
	static void HitCharacter(stringstream& RecvStream, stSOCKETINFO* pSocket);			// 캐릭터 피격 처리

	// 브로드캐스트 함수
	static void Broadcast(stringstream& SendStream);
	// 다른 클라이언트들에게 새 플레이어 입장 정보 보냄
	static void BroadcastNewPlayer(cCharacter& player);
	// 캐릭터 정보를 버퍼에 기록
	static void WriteCharactersInfoToSocket(stSOCKETINFO* pSocket);
};