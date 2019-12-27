#pragma once
#include <iostream>

class ClientInfo
{
public:
	ClientInfo();
	virtual ~ClientInfo();

	void			SetChannelNo(int channel);
	void			SetRoomNo(int room);

	inline __int64 GetUserPid() { return _userPid; }
	inline int		GetChannelNo() { return _channelNo; }
	inline int		GetRoomNo() { return _roomNo; }
	inline std::string		GetUserId() { return _userId; }

private:

	__int64			_userPid;		// contents
	std::string		_userId;		// contents
	int				_channelNo;		// contents
	int				_roomNo = 0;	// contents
};

