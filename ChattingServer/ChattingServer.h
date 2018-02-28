#pragma once
#include "stdafx.h"

// 통신 클래스
class ChattingServer
{
public:
	ChattingServer();
	virtual ~ChattingServer();

private:
	User		mClients[MAX_USER];
	int			clientId = { -1 };
	HANDLE		m_hiocp = { 0 };
	//bool m_b_server_shut_down = { false };

public:	

	void		err_display(char *msg, int err_no);
	void		err_quit(char *msg);

	void		InitServer();
	void		ReleaseServer();

	void		AcceptThread();
	void		WorkerThread();
};

