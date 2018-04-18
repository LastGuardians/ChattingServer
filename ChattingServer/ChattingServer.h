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
	Protocols::PacketType type;
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
	
	inline int					GetClientID() { return clientId; }
	User*						GetUserInfo(int clientId) const;
	inline std::vector<User*>	GetAllUser() { return mClients; }
	
	void		InitServer();
	void		ReleaseServer();
	void		CloseSocket(unsigned long id);
	void		err_display(char *msg, int err_no);

	void		AcceptThread();
	void		WorkerThread();

	//// protobuf 적용 함수 ////
	void		PacketProcess(int id, protobuf::io::CodedInputStream& input_stream);
	void		ProcessCreateRoomPacket(int id, const Protocols::Create_Room message) const; 
	void		ProcessChangeChannelPacket(int id, const Protocols::Change_Channel message) const;
	void		ProcessRoomChattingPacket(int id, const Protocols::Room_Chatting message) const;
	void		ProcessEnterRoomPacket(int id, const Protocols::Enter_Room message) const;
	void		ProcessChannelChattingPacket(int id, const Protocols::Channel_Chatting message) const;
	void		ProcessLeaveRoomPacket(int id, const Protocols::Leave_Room message) const;
	void		ProcessRoomUserListPacket(int id, const Protocols::Room_List message) const;		
	//////////////////////////

	int			WsaRecv(int id);
	int			SendPacket(int id, unsigned char *packet) const;
	void		SendNotifyExistRoomPacket(google::protobuf::int32 id, google::protobuf::int32 room, bool exist) const;
	void		SendRoomChattingPacket(int id, int target, std::string msg, int len) const;
	void		SendEnterRoomPacket(int id, bool enter, int room) const;
	void		SendChannelChattingPacket(int id, int target, std::string msg, int len) const;
	void		SendNotifyEnterRoomPacket(int id, int target) const;
	void		SendNotifyLeaveRoomPacket(int id, int target) const;
	void		SendRoomUserListPacket(int id, int room, int* user, int userCnt) const;
		
};

