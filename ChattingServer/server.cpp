#include "stdafx.h"



int main()
{
	// 서버 초기화
	ChattingServer chatServer;
	bool server_opened = chatServer.InitServer();

	if (true == server_opened)
		chatServer.ServerThreadStart();
}

