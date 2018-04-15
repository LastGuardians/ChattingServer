#include "stdafx.h"

MSODBC::MSODBC()
{
}


MSODBC::~MSODBC()
{
}

void MSODBC::AllocateHandles()
{
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		// ODBC 드라이버 버전 명시
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			// 연결 핸들러 할당
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				printf("Allocate Success\n");
			}
			else {
				SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, ++rec, state, &native, message, sizeof(message), &length);
				printf("%s : %ld : %ld : %s\n", state, rec, native, message);
			}
		}
		else {
			SQLGetDiagRec(SQL_HANDLE_ENV, henv, ++rec, state, &native, message, sizeof(message), &length);
			printf("%s : %ld : %ld : %s\n", state, rec, native, message);
		}
	}
	else {
		SQLGetDiagRec(SQL_HANDLE_ENV, henv, rec, state, &native, message, sizeof(message), &length);
		printf("%s : %ld : %ld : %s\n", state, rec, native, message);
	}
}

void MSODBC::ConnectDataSource()
{
	retcode = SQLConnect(hdbc, (SQLWCHAR*)L"Chatting", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
}

void MSODBC::ExecuteStatementDirect(SQLWCHAR * sql)
{
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
		printf("Connect Success\n");
	}
	else {
		SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, ++rec, state, &native, message, sizeof(message), &length);
		printf("%s : %ld : %ld : %s\n", state, rec, native, message);
	}
	//std::wstring sql_query = std::wstring(sql.begin(), sql.end());
	//const wchar_t* sql_query_wchar = sql_query.c_str();
	retcode = SQLExecDirect(hstmt, sql, SQL_NTS);	

	if (retcode == SQL_SUCCESS) {
		printf("Query Seuccess\n");
	}
	else {
		SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, ++rec, state, &native, message, sizeof(message), &length);
		printf("%s : %ld : %ld : %s\n", state, rec, native, message);
	}
}

void MSODBC::PrepareStatement(SQLWCHAR * sql)
{
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
		printf("Connect Success\n");
	}
	else {
		SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, ++rec, state, &native, message, sizeof(message), &length);
		printf("%s : %ld : %ld : %s\n", state, rec, native, message);
	}
	retcode = SQLPrepare(hstmt, sql, SQL_NTS);

	if (retcode == SQL_SUCCESS) {
		printf("\nQuery Prepare Success\n");
	}
	else {
		SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, ++rec, state, &native, message, sizeof(message), &length);
		printf("\n%s : %ld : %ld : %s\n", state, rec, native, message);
	}
}

void MSODBC::RetrieveResult()
{
	SQLWCHAR user_id[20];
	//float choco_cal;

	SQLLEN uid = 0;

	retcode = SQLBindCol(hstmt, 1, SQL_WCHAR, &user_id, 100, &uid);
	//retcode = SQLBindCol(hstmt, 2, SQL_WCHAR, &Name, NAME_LEN, &cbName);
	//retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &PLevel, PHONE_LEN, &cbPhone);
				
	for (int i = 0; ; i++) {
		retcode = SQLFetch(hstmt);
		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
			std::cout << "error" << std::endl;
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			std::wcout << "[" << i + 1 << "] " << L"user_id: " << user_id
			<< std::endl;
		else
			break;
	}	
	
	SQLFreeStmt(hstmt, SQL_UNBIND);
}
