#include "Socket.h"

Socket::Socket()
{
}

Socket::~Socket()
{
}

bool Socket::Create()
{
	_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
		NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == _socket)
	{
		return false;
	}

	return true;
}

bool Socket::Bind(sockaddr * address)
{
	auto ret = bind(_socket, address, sizeof(address));
	if (SOCKET_ERROR == ret)
		return false;

	return true;
}

bool Socket::Listen()
{
	auto ret = listen(_socket, SOMAXCONN);
	if (SOCKET_ERROR == ret)
		return false;

	return true;
}

SOCKET Socket::Accept(const sockaddr * address)
{
	int addrlen = sizeof(address);
	return accept(_socket, reinterpret_cast<sockaddr *>(&address), &addrlen);
}

bool Socket::Close()
{
	if (INVALID_SOCKET == _socket)
	{
		return false;
	}

	closesocket(_socket);
	return true;
}
