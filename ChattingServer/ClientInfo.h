#pragma once
#include <iostream>
#include <WinSock2.h>

class ClientInfo
{
public:
	ClientInfo();
	virtual ~ClientInfo();

	void	SetChannelNo(int channel);
	void	SetRoomNo(int room);

	int			GetChannelNo() { return _channelNo; }
	int			GetRoomNo() { return _roomNo; }
	__int64		GetUserPid() { return _userPid; }
	std::string	GetUserId() { return _userId; }

private:

	// Contents
	__int64			_userPid;
	std::string		_userId;
	int				_channelNo;
	int				_roomNo;

	// Network
	SOCKET			_userSocket;
};

