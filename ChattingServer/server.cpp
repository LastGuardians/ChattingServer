#include "stdafx.h"



int main()
{
	// ���� �ʱ�ȭ
	ChattingServer chatServer;
	bool server_opened = chatServer.InitServer();

	if (true == server_opened)
		chatServer.ServerThreadStart();
}

