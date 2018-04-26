#pragma once
#include <windows.h>
#include <sqlext.h>
#include <stdio.h>

class ODBC
{
private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt;
	SQLRETURN retcode;
	// For specific error
	SQLSMALLINT length;
	SQLINTEGER rec = 0, native;
	SQLWCHAR state[7], message[256];

public:
	ODBC();
	virtual ~ODBC();

	void AllocateHandles();
	void ConnectDataSource();
	void ExecuteStatementDirect(SQLWCHAR * sql);
	void PrepareStatement(SQLWCHAR* sql);
	void ExecuteStatement();
	void RetrieveResult(int id);
	void DisconnectDataSource();
};

