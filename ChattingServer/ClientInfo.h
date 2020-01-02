#pragma once
#include <iostream>
#include <WinSock2.h>
#include "Socket.h"

class ClientInfo
{
	friend class Socket;

public:
	ClientInfo();
	virtual ~ClientInfo();

	void	SetChannelNo(int channel);
	void	SetRoomNo(int room);

	int			GetChannelNo() { return _channelNo; }
	int			GetRoomNo() { return _roomNo; }
	__int64		GetUserPid() { return _userPid; }
	std::string	GetUserId() { return _userId; }
	Socket*		GetUserSocket() { return &_userSocket; }

private:

	// Contents
	__int64			_userPid;
	std::string		_userId;
	int				_channelNo;
	int				_roomNo;

	// Network	
	Socket			_userSocket;
};

