#include "stdafx.h"


User::User() : userSocket{0}, userId{-1}
{

}

User::User(SOCKET s, bool connect, int id)
{

}


User::~User()
{

}

void User::SetUserInfo(SOCKET s, bool connect, int id)
{
	userSocket = s;
	userConnected = connect;
	userId = id;
}

int User::SendPacket(unsigned char *packet)
{
	Overlap *over = new Overlap;
	memset(over, 0, sizeof(Overlap));
	over->operation = OV_SEND;
	over->wsabuf.buf = reinterpret_cast<CHAR *>(over->iocp_buff);
	over->wsabuf.len = packet[0];
	memcpy(over->iocp_buff, packet, packet[0]);

	int ret = WSASend(userSocket, &over->wsabuf, 1, NULL, 0,
		&over->overlap, NULL);
	return ret;
}

int User::WsaRecv(unsigned char *packet)
{

}