#pragma once

// 유저 클래스 (소켓)
class User
{
public:
	User();
	User(SOCKET s, bool connect, int id);
	virtual ~User();

private:
	SOCKET			userSocket;		// network
	int				userId;			// contents
	int				channel_index;	// contents
	int				room_index = 0;  // contents

public:
	void			SetUserInfo(SOCKET s, bool connect, int id);
	//int				PacketRessembly(int id, DWORD packetSize);
	void			SetChannelIndex(int channel);
	void			SetRoomIndex(int room);

	inline int		GetChannelIndex() { return channel_index; }
	inline int		GetRoomIndex() { return room_index; }
	inline int		GetUserId() { return userId; }
	inline SOCKET	GetUserSocket() { return userSocket; }
};

