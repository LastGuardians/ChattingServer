#include "Socket.h"


CSocket::CSocket()
{
}

CSocket::~CSocket()
{
}

bool CSocket::Create()
{
	_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
		NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == _socket)
	{
		return false;
	}

	return true;
}

bool CSocket::Bind(SOCKADDR_IN& addr)
{
	auto ret = bind(_socket, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
	if (SOCKET_ERROR == ret)
		return false;

	return true;
}

bool CSocket::Listen()
{
	auto ret = listen(_socket, SOMAXCONN);
	if (SOCKET_ERROR == ret)
		return false;

	return true;
}

SOCKET CSocket::Accept(std::string& addr)
{
	int addrlen = sizeof(addr);
	return WSAAccept(_socket, reinterpret_cast<sockaddr *>(&addr), &addrlen, NULL, NULL);
}

bool CSocket::Close()
{
	if (INVALID_SOCKET == _socket)
	{
		return false;
	}

	closesocket(_socket);
	return true;
}
