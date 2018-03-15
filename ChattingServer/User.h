#pragma once

struct Overlap
{
	WSAOVERLAPPED	overlap = { 0 };
	int				event_type = { OV_RECV };
	WSABUF			wsabuf = { 0 };
	unsigned char	iocp_buff[BUFSIZE] = { 0 };
};

struct RecvBuffInfo {
	unsigned char	buf[BUFSIZE];
	int				sizePrev = { 0 };
	int				sizeCurr = { 0 };
};

// 유저 클래스 (소켓)
class User
{
	friend class ChannelManager;
	friend class Room;

public:
	User();
	User(int id);
	User(SOCKET s, bool connect, int id);
	virtual ~User();

private:
	SOCKET			userSocket;
	bool			userConnected;
	int				userId;
	int				channel_index;
	int				room_index;

	Overlap			recv_over;
	RecvBuffInfo	recv_buff;

	ChannelManager*	_channelManager;	
	Room*			_roomMananger;			// 룸 매니저 객체

public:
	void			SetUserInfo(SOCKET s, bool connect, int id);
	int				PacketRessembly(int id, DWORD packetSize);
	void			SetChannelIndex(int channel);
	void			SetRoomIndex(int room);

	inline int		GetChannelIndex() { return channel_index; }
	inline int		GetRoomIndex() { return room_index; }
	inline int		GetUserId() { return userId; }
	inline SOCKET	GetUserSocket() { return userSocket; }

	void		ProcessPacket(int id, unsigned char *buf);
	void		ProcessEneterChannelPacket(int id, unsigned char *buf);
	void		ProcessLeaveChannelPacket(int id, unsigned char *buf);
	void		ProcessCreateRoomPacket(int id, unsigned char *buf);
	void		ProcessChangeChannelPacket(int id, unsigned char *buf);
	void		ProcessRoomChattingPacket(int id, unsigned char *buf);
	void		ProcessEnterRoomPacket(int id, unsigned char *buf);

	int			WsaRecv();
	int			SendPacket(unsigned char *packet);
	void		SendNotifyExistRoomPacket(int room, bool exist);
	void		SendRoomListPacket();

	void		CloseSocket();

};

