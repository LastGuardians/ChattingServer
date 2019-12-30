#pragma once
#include <WinSock2.h>

struct OVERLAP
{
	OVERLAPPED		overlap = { 0 };
	int				event_type = { OV_RECV };
	WSABUF			wsabuf = { 0 };
	unsigned char	iocp_buff[BUFSIZE] = { 0 };
};

typedef OVERLAP IOContext;

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

