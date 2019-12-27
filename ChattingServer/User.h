#pragma once

// 유저 클래스 (소켓)
class User
{
public:
	User();
	User(SOCKET s, int id);
	virtual ~User();

private:
	SOCKET			_userSocket;		// network
	int				_userId;			// contents
	int				_channel_index;		// contents
	int				_room_index = 0;	// contents

public:
	
	void			SetChannelIndex(int channel);
	void			SetRoomIndex(int room);

	inline int		GetChannelIndex() { return _channel_index; }
	inline int		GetRoomIndex() { return _room_index; }
	inline int		GetUserId() { return _userId; }
	inline SOCKET	GetUserSocket() { return _userSocket; }

};

