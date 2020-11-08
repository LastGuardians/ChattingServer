#include "stdafx.h"
#include "Socket.h"

IMPLEMENT_SINGLETON(CNetwork);

CNetwork::CNetwork()
{
}

CNetwork::~CNetwork()
{
}

bool CNetwork::Start(CSocket* socket)
{
	if (false == socket->Create())
	{
		return false;
	}

	SOCKADDR_IN localAddr;
	if (false == GetLocalHostAddr(localAddr))
	{
		return false;
	}

	if (false == socket->Bind(localAddr))
	{
		return false;
	}

	if (false == socket->Listen())
	{
		return false;
	}

	return true;
}

void CNetwork::Stop()
{
	
}

bool CNetwork::Send(CSocket* handle, google::protobuf::Message * msg)
{
	//WSABUF wsaBuffer;
	//wsaBuffer.buf = msg;

	//int ret = WSASend(handle, &wsaBuffer, 1, NULL, 0,
	//	&over->overlap, NULL);
	return false;
}

bool CNetwork::SendBroadcast(google::protobuf::Message * msg)
{
	return true;
}

bool CNetwork::Close(SOCKET* handle)
{
	closesocket(*handle);
	return true;
}

bool CNetwork::GetLocalHostAddr(SOCKADDR_IN &OUT addr)
{
	char hostname[5120];
	bool gethostfailed = (0 != ::gethostname(hostname, sizeof(hostname)));
	if (false == gethostfailed) return false;

	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9000);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	return true;
}
