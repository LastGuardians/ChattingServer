#include "stdafx.h"
#include "Socket.h"

Acceptor::Acceptor()
{
}

Acceptor::~Acceptor()
{
	Stop();
}

bool Acceptor::Start()
{
	if (false == _socket->Create())
	{
		Stop();
		return false;
	}

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (_socket->Bind(reinterpret_cast<sockaddr*>(&serveraddr)) == SOCKET_ERROR)
	{
		Stop();
		return false;
	}

	if (_socket->Listen() == SOCKET_ERROR)
	{
		Stop();
		return false;
	}

	return true;
}

bool Acceptor::Stop()
{
	_socket->Close();
	return true;
}


