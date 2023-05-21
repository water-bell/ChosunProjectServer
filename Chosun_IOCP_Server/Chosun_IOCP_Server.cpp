// Chosun_IOCP_Server.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "stdafx.h"
#include "Main_IOCP.h"

int main()
{
	MainIOCP iocp_server;
	if (iocp_server.Initialize())
	{
		iocp_server.StartServer();
	}
	return 0;
}


