#pragma once

struct Overlap
{
	WSAOVERLAPPED	overlap;
	int				operation;
	WSABUF			wsabuf;
	unsigned char	iocp_buff[BUFSIZE];
};

// ���� Ŭ���� (����)
class User
{
public:
	User();
	User(SOCKET s, bool connect, int id);
	virtual ~User();

private:
	SOCKET		userSocket;
	bool		userConnected;
	int			userId;

	Overlap		recv_over;

public:
	void		SetUserInfo(SOCKET s, bool connect, int id);
	int			SendPacket(unsigned char *packet);
	int			WsaRecv(unsigned char *packet);
};

