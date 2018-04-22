#pragma once

// 유저 클래스 (소켓)
class User
{
public:
	User();
	User(SOCKET s, bool connect, int id);
	virtual ~User();

private:
	SOCKET			userSocket;
	bool			userConnected;
	bool			loginSuccess;
	int				userId;
	int				channel_index;
	int				room_index = 0;
	char			user_name[MSG_SIZE];

public:
	
	void			SetChannelIndex(int channel);
	void			SetRoomIndex(int room);
	void			SetLoginSuccess(bool status);
	void			SetUserName(char* name, int len);

	inline int		GetChannelIndex() { return channel_index; }
	inline int		GetRoomIndex() { return room_index; }
	inline int		GetUserId() { return userId; }
	inline SOCKET	GetUserSocket() { return userSocket; }
	inline bool		GetUserLoginStatus() { return loginSuccess; }
	inline char*	GetUserNameID() { return user_name; }

};

