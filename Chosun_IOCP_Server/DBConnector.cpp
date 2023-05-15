#include "stdafx.h"
#include "DBConnector.h"

DBConnector::DBConnector()
{
}

DBConnector::~DBConnector()
{
}

bool DBConnector::Connect(
	const string& Server,
	const string& User,
	const string& Password,
	const string& Database,
	const int& Port )
{
	Conn = mysql_init(NULL);
	if (!mysql_real_connect(Conn, Server.c_str(), User.c_str(), Password.c_str(), Database.c_str(), Port, NULL, 0))
		printf_s("[DB] DB 접속 실패\n"); return FALSE;

	return TRUE;
}

void DBConnector::Close()
{
	mysql_close(Conn);
}

bool DBConnector::SearchAccount(const string& Id, const string& Password)
{
	bool bResult = FALSE;
	string sql; // DB 생성 이후 SELECT sql문 작성

	if (mysql_query(Conn, sql.c_str()))
		printf_s("[DB] 검색 실패\n"); return FALSE;

	Res = mysql_use_result(Conn);
	Row = mysql_fetch_row(Res);

	if (Row != NULL)	
		bResult = TRUE;
	else				
		printf_s("[ERROR] 일치하는 ID 없음\n"); 

	mysql_free_result(Res);

	return bResult;
}

bool DBConnector::SingUpAccount(const string& Id, const string& Password)
{
	bool bResult = FALSE;

	string sql; // DB 생성 이후 INSERT sql문 작성

	if (mysql_query(Conn, sql.c_str()))
		printf_s("[DB] 중복된 계정\n"); return FALSE;

	bResult = TRUE;

	return bResult;
}