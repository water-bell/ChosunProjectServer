// Chosun_IOCP_Client.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <iostream>
#include <sstream>
#include <map>
#include "CommonClass.h"

using namespace std;

#define	MAX_BUFFER		4096
#define SERVER_PORT		25000
#define SERVER_IP		"127.0.0.1"
#define MAX_CLIENTS		100

struct stSOCKETINFO
{
	WSAOVERLAPPED	overlapped;
	WSABUF			dataBuf;
	SOCKET			socket;
	char			messageBuffer[MAX_BUFFER];
	int				recvBytes;
	int				sendBytes;
};

int main()
{
	WSADATA wsaData;
	//윈속 초기화
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nRet != 0)
	{
		cout << "Error : " << WSAGetLastError() << endl;
		return FALSE;
	}

	//통신 소켓 생성
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "Error : " << WSAGetLastError() << endl;
		return FALSE;
	}
	
	cout << "소켓 초기화 성공" << endl;

	SOCKADDR_IN stServerAddr;

	char szBuffer[MAX_BUFFER];

	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(SERVER_PORT);
	stServerAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	//포트 바인딩 및 연결
	nRet = connect(clientSocket, (sockaddr*)&stServerAddr, sizeof(sockaddr));
	if (nRet == SOCKET_ERROR)
	{
		printf_s("Error[%d]: 서버에 연걸할 수 없습니다.\n", WSAGetLastError());
		return FALSE;
	}

	printf_s("서버에 연결되었습니다.\n");

	stringstream SendStream;
	SendStream << EPacketType::SIGNUP << endl;
	SendStream << "testId3" << endl; //인게임에서는 textbox에 입력한 내용 받아와 로그인 버튼 누르면 함수 실행
	SendStream << "testPw3" << endl;

	send(clientSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0);
	recv(clientSocket, szBuffer, MAX_BUFFER, 0);

	stringstream RecvStream;
	bool result;
	int packetType;

	RecvStream << szBuffer;
	RecvStream >> packetType;

	if (!(RecvStream >> result))
	{
		cout << "Error: 결과 값을 불러오지 못했습니다." << endl;
	}
	
	switch (packetType)
	{
	case EPacketType::SIGNUP:
		if (result)
			printf_s("회원가입되었습니다.\n");
		else
			printf_s("이미 존재하는 아이디입니다.\n");
		break;
	case EPacketType::LOGIN:
		if (result)
			printf_s("로그인 성공.\n");
		else
			printf_s("존재하지 않는 아이디입니다.\n");
		break;
	}


	closesocket(clientSocket);
	WSACleanup();
	printf_s("클라이언트가 종료되었습니다.\n");

	return 0;
}

