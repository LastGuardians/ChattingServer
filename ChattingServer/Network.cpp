#include "stdafx.h"

IMPLEMENT_SINGLETON(CNetwork);

CNetwork::CNetwork()
{
}

CNetwork::~CNetwork()
{
}

bool CNetwork::Start()
{
	return _acceptor.Start();
}

void CNetwork::Stop()
{
	_acceptor.Stop();
}

bool CNetwork::Send(SOCKET handle, google::protobuf::Message * msg)
{
	WSABUF wsaBuffer;
	//wsaBuffer.buf = msg;

	//int ret = WSASend(handle, &wsaBuffer, 1, NULL, 0,
	//	&over->overlap, NULL);
	return false;
}

bool CNetwork::SendBroadcast(google::protobuf::Message * msg)
{
	return false;
}

bool CNetwork::Close(SOCKET handle)
{
	return false;
}
