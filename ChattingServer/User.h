#pragma once

// 유저 클래스 (소켓)
class User
{
public:
	User();
	User(SOCKET s, bool connect, int id);
	virtual ~User();

private:
	SOCKET			_userSocket;
	bool			_userConnected;
	int				_userId;
	int				_channel_index;
	int				_room_index = 0;

public:
	
	void			SetChannelIndex(int channel);
	void			SetRoomIndex(int room);

	inline int		GetChannelIndex() { return _channel_index; }
	inline int		GetRoomIndex() { return _room_index; }
	inline int		GetUserId() { return _userId; }
	inline SOCKET	GetUserSocket() { return _userSocket; }

};

