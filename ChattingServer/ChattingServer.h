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

struct MessageHeader
{
	protobuf::uint32 size;
};
const int MessageHeaderSize = sizeof(MessageHeader);


// 통신 클래스
class ChattingServer 
{
public:
	ChattingServer();
	virtual ~ChattingServer();

	DECLARE_SINGLETON(ChattingServer);

private:
	std::vector<User*>	mClients;
	int					clientId = { -1 };
	HANDLE				m_hiocp = { 0 };
	bool				m_b_server_shut_down = { false };

	Overlap				recv_over;
	RecvBuffInfo		recv_buff;


public:	

	void						err_display(char *msg, int err_no);
	
	void						SetUserInfo(User* user);
	inline int					GetClientID() { return clientId; }
	User*						GetUserInfo(int clientId);
	inline std::vector<User*>	GetAllUser() { return mClients; }

	void		InitServer();
	void		ReleaseServer();
	void		CloseSocket(int id);

	void		AcceptThread();
	void		WorkerThread();

	int			PacketRessembly(int id, DWORD packetSize);
	void		ProcessPacket(int id, unsigned char *buf);
	void		PacketProcess(protobuf::io::CodedInputStream& input_stream, const ChattingServer& handler);
	void		ProcessEneterChannelPacket(int id, unsigned char *buf);
	void		ProcessLeaveChannelPacket(int id, unsigned char *buf);
	void		ProcessCreateRoomPacket(int id, unsigned char *buf);
	void		ProcessChangeChannelPacket(int id, unsigned char *buf);
	void		ProcessRoomChattingPacket(int id, unsigned char *buf);
	void		ProcessEnterRoomPacket(int id, unsigned char *buf);

	int			WsaRecv(int id);
	int			SendPacket(int id, unsigned char *packet);
	void		SendNotifyExistRoomPacket(int id, int room, bool exist);
	void		SendRoomListPacket();
	void		SendRoomChattingPacket(int id, int target, char* msg, int len);
	void		SendEnterRoomPacket(int id, bool enter, int room);
		
};

