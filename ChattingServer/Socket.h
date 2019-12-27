#pragma once
#include <WinSock2.h>

class Socket
{
public:
	Socket();
	virtual ~Socket();
	
	bool Create();
	bool Bind(sockaddr *address);
	bool Listen();
	SOCKET Accept(const sockaddr *address);
	bool Close();

private:

	SOCKET _socket;
};

