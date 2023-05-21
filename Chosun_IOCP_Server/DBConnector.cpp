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
	{
		printf_s("[DB] DB ���� ����\n");
		return FALSE;
	}
	

	return TRUE;
}

void DBConnector::Close()
{
	mysql_close(Conn);
}

bool DBConnector::SearchAccount(const string& Id, const string& Password)
{
	bool bResult = FALSE;
	string sql = "SELECT * FROM account_info.user_account where id = '"; 
	sql += Id + "' and pw = '" + Password + "'";
	if (mysql_query(Conn, sql.c_str()))
	{
		printf_s("[DB] �˻� ����\n");
		return FALSE;
	}
		
	Res = mysql_use_result(Conn);
	Row = mysql_fetch_row(Res);

	if (Row != NULL)	
		bResult = TRUE;
	else
	{
		printf_s("[ERROR] ��ġ�ϴ� ID ����\n");
		bResult = FALSE;
	}
	mysql_free_result(Res);

	return bResult;
}

bool DBConnector::SignUpAccount(const string& Id, const string& Password)
{
	bool bResult = FALSE;

	string sql = "INSERT INTO account_info.user_account (id,pw) VALUES"; 
	sql += " ('" + Id + "', '" + Password + "')";

	if (mysql_query(Conn, sql.c_str()))
	{
		printf_s("[DB] �ߺ��� ����\n");
		return FALSE;
	}

	printf_s("[DB] ȸ�� ���� ����\n");
	bResult = TRUE;

	return bResult;
}