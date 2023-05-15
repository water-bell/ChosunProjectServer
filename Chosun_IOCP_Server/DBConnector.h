#pragma once
#include "mysql.h"
#include <string>

using namespace std;

class DBConnector
{
public:
	DBConnector();
	~DBConnector();

	bool Connect(
		const string&	Server,
		const string&	User,
		const string&	Password,
		const string&	Database,
		const int&		Port
	);

	void Close();

	bool SearchAccount(const string& Id, const string& Password);
	bool SingUpAccount(const string& Id, const string& Password);

private:
	MYSQL*		Conn;
	MYSQL_RES*	Res;
	MYSQL_ROW	Row;
};