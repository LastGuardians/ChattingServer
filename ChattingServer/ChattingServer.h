#pragma once
#include "stdafx.h"

// ��� Ŭ����
class ChattingServer
{
public:
	ChattingServer();
	virtual ~ChattingServer();

private:
	vector<User*> m_clients;
	int clientId;

public:	

	void err_display(char *msg, int err_no);
	void err_quit(char *msg);

	void AcceptThread();
	void WorkerThread();
};

