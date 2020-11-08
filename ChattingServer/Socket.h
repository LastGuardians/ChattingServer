#pragma once
/*
	SOCKET ·¦ÇÎ class
*/
#include <WinSock2.h>
#include <string>

class CSocket
{
public:
	CSocket();
	virtual ~CSocket();
	
	bool Create();
	bool Bind(SOCKADDR_IN& addr);
	bool Listen();
	SOCKET Accept(std::string& addr);
	bool Close();

private:

	SOCKET _socket;
};

