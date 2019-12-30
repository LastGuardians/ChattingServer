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

// 패킷 헤더
struct MessageHeader
{
	int type;
	//Protocols::PacketType type;
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

	friend class Channel;

private:
	std::unordered_map<int, User*>	_mClients;
	int								_clientId = { -1 };
	HANDLE							_hiocp = { 0 };
	bool							_server_shut_down = { false };
	mutable std::mutex				_cs_lock;

	Overlap							_recv_over;
	std::vector<Channel*>			_channel;

public:	
	User*		GetUserInfo(int id) const;
	
	bool		InitServer();
	void		ServerThreadStart();
	void		CloseSocket(unsigned long id);
	void		err_display(char *msg, int err_no);	

	void		AcceptThread();
	void		WorkerThread();

	//// protobuf 적용 함수 ////
	void		PacketProcess(int id, protobuf::io::CodedInputStream& input_stream);
	void		ProcessCreateRoomPacket(int id, const Protocols::Create_Room& message) const; 
	void		ProcessChangeChannelPacket(int id, const Protocols::Change_Channel& message) const;
	void		ProcessRoomChattingPacket(int id, const Protocols::Room_Chatting& message) const;
	void		ProcessEnterRoomPacket(int id, const Protocols::Enter_Room& message) const;
	void		ProcessChannelChattingPacket(int id, const Protocols::Channel_Chatting& message) const;
	void		ProcessLeaveRoomPacket(int id, const Protocols::Leave_Room& message) const;
	void		ProcessRoomUserListPacket(int id, const Protocols::Room_List& message) const;		
	//////////////////////////

	int			SendPacket(int id, unsigned char *packet) const;
	int			SendPacketAssemble(int id, int type, google::protobuf::Message& msg) const;
		
};

