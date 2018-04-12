#include "stdafx.h"

IMPLEMENT_SINGLETON(ChattingServer);

ChattingServer::ChattingServer()
{
	recv_over.wsabuf.buf = reinterpret_cast<CHAR*>(recv_over.iocp_buff);
	recv_over.wsabuf.len = sizeof(recv_over.iocp_buff);	
}

ChattingServer::~ChattingServer()
{
}


User* ChattingServer::GetUserInfo(int clientId) const
{
	for (auto iter : mClients)
	{
		if (iter->GetUserId() == clientId)
		{
			return iter;
			break;
		}
	}

	return nullptr;
}

// 서버 초기화
void ChattingServer::InitServer()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	m_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);
	if (NULL == m_hiocp) { err_display("InitServer() : ", WSAGetLastError()); }

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int m_cpu_core = static_cast<int>(si.dwNumberOfProcessors) / 2;

	// accept 스레드 생성
	std::thread acceptThread{ &ChattingServer::AcceptThread , this };

	// worker 스레드 생성
	std::vector<std::thread *> workerThreads;
	for (int i = 0; i < m_cpu_core * 2; ++i)
	{
		workerThreads.emplace_back(new std::thread{ &ChattingServer::WorkerThread, this });
	}

	for (auto thread : workerThreads) {
		thread->join();
		delete thread;
	}

	acceptThread.join();

	
}

void ChattingServer::ReleaseServer()
{

}

void ChattingServer::CloseSocket(int id)
{
	User *userInfo = GetUserInfo(id);

	Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].DeleteUserToChannel(userInfo);
	Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].DeleteUserToRoom(userInfo->GetRoomIndex(), userInfo);
	userInfo->SetChannelIndex(-1);
	userInfo->SetRoomIndex(-1);
	std::cout << "[ " <<id << " ] User CloseSocket!" << std::endl;
	
	SOCKET sock = userInfo->GetUserSocket();
		
	for (auto iter = mClients.begin(); iter != mClients.end(); ++iter)
	{
		auto info = *iter;
		if (info->GetUserId() == id)
		{
			mClients.erase(iter);
			break;
		}
	}

	closesocket(sock);
}

void ChattingServer::err_display(char *msg, int err_no)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);
	printf("%s\n", msg, (LPCTSTR)lpMsgBuf);
	printf("err_no : %d\n", err_no);
	LocalFree(lpMsgBuf);
}

void ChattingServer::AcceptThread()
{
	int retval;

	//socket()
	SOCKET listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
									NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == listen_sock) { err_display("WSASocket() : ", WSAGetLastError()); };

	//bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = ::bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_display("bind() : ", WSAGetLastError());

	//listen() 
	retval = ::listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_display("listen() : ", WSAGetLastError());

	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;

	while (1)
	{
		//accept()
		addrlen = sizeof(clientaddr);
		SOCKET client_sock = WSAAccept(listen_sock, reinterpret_cast<sockaddr *>(&clientaddr), &addrlen, NULL, NULL);
		if (client_sock == INVALID_SOCKET) {
			err_display("WSAAccept() : ", WSAGetLastError());
			break;
		}

		printf("\n[Server] Client Connect : IP = %s, Port = %d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		if (INVALID_SOCKET == client_sock)
		{
			int error = WSAGetLastError();
			err_display("Accept::WSAAccept Error\n", error);
			while (true);
		}

		if (clientId > 50) {
			std::cout << "Max User Connect!" << std::endl;
		}
		clientId += 1;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_sock), m_hiocp, clientId, 0);
				
		User* userInfo = new User(client_sock, true, clientId);
		mClients.push_back(userInfo);
		
		srand(time(NULL));
		// 접속한 클라이언트에게 랜덤한 채널 인덱스 부여 = 접속과 동시에 채널 입장
		Protocols::Enter_Channel enter;
		enter.set_id(clientId);
		enter.set_channelindex(rand() % 5);

		size_t bufSize = enter.ByteSizeLong();
		char* outputBuf = new char[bufSize];

		// 헤더 생성
		MessageHeader header;
		header.size = MessageHeaderSize + bufSize;
		header.type = Protocols::ENTER_CHANNEL;
		char* header_seri = reinterpret_cast<char*>(&header);

		int rtn = enter.SerializeToArray(outputBuf, bufSize);

		// 전송 버퍼 생성
		unsigned char* resultBuf = new unsigned char[bufSize + MessageHeaderSize];
		memcpy(resultBuf, header_seri, MessageHeaderSize);
		memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);

		userInfo->SetChannelIndex(enter.channelindex());
		userInfo->SetRoomIndex(-1);

		// 채널에 유저 입장		
		Singleton::GetInstance()->channel[enter.channelindex()].AddUserToChannel(userInfo);

		int result = SendPacket(clientId, resultBuf);
		if (SOCKET_ERROR == result) {
			if (ERROR_IO_PENDING != WSAGetLastError()) {
				err_display("Accept::WSASend Error! : ", WSAGetLastError());
			}
		} 
				
		retval = WsaRecv(clientId);	
		if (SOCKET_ERROR == retval)
		{
			if (ERROR_IO_PENDING != WSAGetLastError()) {
				err_display("Accept::WSARecv Error! : ", WSAGetLastError());
			}
		}
	}
}

void ChattingServer::WorkerThread()
{
	while (false == m_b_server_shut_down)
	{
		unsigned long long id = { 0 };
		DWORD io_size = { 0 };
		Overlap *ovlp = { nullptr };

		BOOL result = GetQueuedCompletionStatus(m_hiocp, &io_size, (PULONG_PTR)&id,
			reinterpret_cast<LPOVERLAPPED*>(&ovlp), INFINITE);
		if (FALSE == result) {
			int error = WSAGetLastError();
			if (WSA_IO_PENDING != error)
				err_display("GetQueuedCompletionStatus error!\n", error);
		}

		if (0 == io_size) {
			// 접속 종료한 클라 disconnect
			CloseSocket(id);
			continue;
		}
		if (OV_RECV == ovlp->event_type) {
			protobuf::io::ArrayInputStream input_array_stream(recv_over.iocp_buff, BUFSIZE);
			protobuf::io::CodedInputStream input_coded_stream(&input_array_stream);

			// 패킷 분석
			PacketProcess(id, input_coded_stream);
			int retval = WsaRecv(id);
			if (SOCKET_ERROR == retval) {
				int err_no = WSAGetLastError();
				if (ERROR_IO_PENDING != err_no) { err_display("WorkerThread::WSARecv", err_no); }
			}
			continue;
		}
		else if (OV_SEND == ovlp->event_type) {
			delete ovlp;
			continue;
		}
		else {
			printf("Unknown IOCP event !!\n");
			exit(-1);
		}
	}
}

//int ChattingServer::PacketRessembly(int id, DWORD packetSize)
//{
//	unsigned char *buf_ptr = recv_over.iocp_buff;
//	ProcessPacket(id, buf_ptr);
//
//	return WsaRecv(id);
//}
//
//void ChattingServer::ProcessPacket(int id, unsigned char * buf)
//{
//	switch (buf[0])
//	{
//	case ENTER_CHANNEL:
//		break;
//	case CREATE_ROOM:
//		ProcessCreateRoomPacket(id, buf);
//		break;
//	case CHANGE_CHANNEL:
//		ProcessChangeChannelPacket(id, buf);
//		break;
//	case ROOM_CHATTING:
//		ProcessRoomChattingPacket(id, buf);
//		break;
//	case ENTER_ROOM:
//		ProcessEnterRoomPacket(id, buf);
//		break;
//	case ROOM_LIST:
//		ProcessRoomUserListPacket(id, buf);
//		break;
//	case CHANNEL_CHATTING:
//		ProcessChannelChattingPacket(id, buf);
//		break;
//	case LEAVE_ROOM:
//		ProcessLeaveRoomPacket(id, buf);
//		break;
//	default:
//		break;
//	}
//}

void ChattingServer::PacketProcess(int id, protobuf::io::CodedInputStream & input_stream)
{
	MessageHeader messageHeader;
	
	// 헤더를 읽어냄
	while (input_stream.ReadRaw(&messageHeader, MessageHeaderSize))
	{
		// 직접 억세스 할수 있는 버퍼 포인터와 남은 길이를 알아냄
		const void* payload_ptr = NULL;
		int remainSize = 0;
		input_stream.GetDirectBufferPointer(&payload_ptr, &remainSize);
		if (remainSize < (signed)messageHeader.size)
			break;

		// 메세지 본체를 읽어내기 위한 스트림을 생성
		protobuf::io::ArrayInputStream payload_array_stream(payload_ptr, messageHeader.size);
		protobuf::io::CodedInputStream payload_input_stream(&payload_array_stream);

		// 메세지 본체 사이즈 만큼 포인터 전진
		input_stream.Skip(messageHeader.size);

		// 메세지 종류별로 역직렬화해서 적절한 메서드를 호출해줌
		switch (messageHeader.type)
		{
		case Protocols::CREATE_ROOM: {
			Protocols::Create_Room message;
		
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessCreateRoomPacket(id, message);
			break;
		}
		case Protocols::CHANGE_CHANNEL: {
			Protocols::Change_Channel message;
			
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessChangeChannelPacket(id, message);
			break;
		}
		case Protocols::ROOM_CHATTING: {
			Protocols::Room_Chatting message;
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessRoomChattingPacket(id, message);
			break;
		}
		case Protocols::ENTER_ROOM: {
			Protocols::Enter_Room message;
			
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessEnterRoomPacket(id, message);
			break;
		}
		case Protocols::ROOM_LIST: {
			Protocols::Room_List message;
			
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessRoomUserListPacket(id, message);
			break;
		}
		case Protocols::CHANNEL_CHATTING: {
			Protocols::Channel_Chatting message;
			
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessChannelChattingPacket(id, message);
			break;
		}
		case Protocols::LEAVE_ROOM: {
			Protocols::Leave_Room message;
		
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessLeaveRoomPacket(id, message);
			break;
		}
		}
	}
}

void ChattingServer::ProcessCreateRoomPacket(int id, const Protocols::Create_Room message) const
{
	User *userInfo = GetUserInfo(id);

	// 방이 하나도 없을 때는 바로 생성한다.
	if (true == Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].GetRoomIsEmpty())
	{
		SendNotifyExistRoomPacket(id, message.roomindex(), false);

		Room *newRoom = new Room(message.roomindex(), userInfo->GetChannelIndex());	// 새로운 방 생성
		Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].AddNewRoom(message.roomindex(), newRoom);  // 채널에 방 추가
		Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].AddUserToRoom(message.roomindex(), userInfo);	// 방에 유저 추가
		userInfo->SetRoomIndex(message.roomindex());

		std::cout << "[" << id << "] 유저가 " << "[" << message.roomindex()
			<< "] 번 방을 생성하였습니다." << std::endl;
	}

	// 방이 하나 이상일 때
	else
	{
		// 이미 존재하는 방인지 체크한다. 이미 존재하면 방 생성을 하지 않는다.
		if (true == Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].GetRoomIsExist(message.roomindex()))
		{
			std::cout << message.roomindex() << "번 방이 이미 있습니다." << std::endl;

			SendNotifyExistRoomPacket(id, message.roomindex(), true);
			return;
		}

		// 존재하는 방이 없을 때
		Room *newRoom = new Room(message.roomindex(), userInfo->GetChannelIndex());	// 새로운 방 생성
		Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].AddNewRoom(message.roomindex(), newRoom); // 채널에 방 추가
		Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].AddUserToRoom(message.roomindex(), userInfo);	// 방에 유저 추가
		userInfo->SetRoomIndex(message.roomindex());

		SendNotifyExistRoomPacket(id, message.roomindex(), false);

		std::cout << "[" << id << "] 유저가 " << "[" << message.roomindex()
			<< "] 번 방을 생성하였습니다." << std::endl;
	}
}

void ChattingServer::ProcessChangeChannelPacket(int id, const Protocols::Change_Channel message) const
{
	User *userInfo = GetUserInfo(id);
	int _currentChannelIndex = userInfo->GetChannelIndex();
	
	Singleton::GetInstance()->channel[_currentChannelIndex].DeleteUserToChannel(userInfo);
	Singleton::GetInstance()->channel[message.channelindex()].AddUserToChannel(userInfo);

	userInfo->SetChannelIndex(message.channelindex());

	std::cout << "[" << id << "] 유저가 " << "[" << message.channelindex()
		<< "] 번 채널로 이동했습니다." << std::endl;
}

void ChattingServer::ProcessChannelChattingPacket(int id, const Protocols::Channel_Chatting message) const
{
	User *userInfo = GetUserInfo(id);
	for (auto iter : mClients)
	{
		//if (iter->GetUserId() == id) continue;
		if (iter->GetChannelIndex() == userInfo->GetChannelIndex())		// 채널이 같으면.
		{
			SendChannelChattingPacket(id, iter->GetUserId(), message.message(), message.ByteSize());
		}
	}
}

void ChattingServer::ProcessRoomChattingPacket(int id, const Protocols::Room_Chatting message) const
{
	User *userInfo = GetUserInfo(id);
	for (auto iter : mClients)
	{
		//if(iter->GetUserId() == id) continue;								// 내 자신은 건너뛴다.
		if (iter->GetChannelIndex() == userInfo->GetChannelIndex())		// 채널이 같고
		{
			if (iter->GetRoomIndex() == userInfo->GetRoomIndex())		// 방이 같아야 메시지를 보낸다.
			{
				SendRoomChattingPacket(id, iter->GetUserId(), message.message(), message.ByteSize());
			}
		}
	}
}

void ChattingServer::ProcessEnterRoomPacket(int id, const Protocols::Enter_Room message) const
{
	User *userInfo = GetUserInfo(id);
	// 방이 존재한다면 바로 입장.
	if (true == Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].GetRoomIsExist(message.roomindex()))
	{
		std::cout << "[" << id << "] 유저가 " << "[" << message.roomindex()
			<< "] 번 방에 입장하였습니다." << std::endl;

		Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].AddUserToRoom(message.roomindex(), userInfo);
		userInfo->SetRoomIndex(message.roomindex());

		// 방 입장 성공
		SendEnterRoomPacket(id, true, message.roomindex());

		for (auto iter : mClients)
		{
			if (iter->GetUserId() == id) continue;
			if (iter->GetChannelIndex() == userInfo->GetChannelIndex())		// 채널이 같고
			{
				if (iter->GetRoomIndex() == userInfo->GetRoomIndex())		// 방이 같아야 한다.
				{
					SendNotifyEnterRoomPacket(id, iter->GetUserId());			// 방 입장 패킷을 보낸다.
				}
			}
		}
		return;
	}

	// 방 입장 실패 
	std::cout << message.roomindex() << " 번 방이 없습니다." << std::endl;
	SendEnterRoomPacket(id, false, message.roomindex());
}

void ChattingServer::ProcessLeaveRoomPacket(int id, const Protocols::Leave_Room message) const
{
	User *userInfo = GetUserInfo(id);
	Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].DeleteUserToRoom(message.roomindex(), userInfo);
	userInfo->SetRoomIndex(0);		// 유저 방 초기화

	for (auto iter : mClients)
	{
		if (iter->GetUserId() == id) continue;
		if (iter->GetChannelIndex() == userInfo->GetChannelIndex())		// 채널이 같고
		{
			if (iter->GetRoomIndex() == message.roomindex())				// 방이 같아야 한다.
			{
				SendNotifyLeaveRoomPacket(id, iter->GetUserId());			// 방 퇴장 패킷을 보낸다.
			}
		}
	}
}

void ChattingServer::ProcessRoomUserListPacket(int id, const Protocols::Room_List message) const
{
	User *user = GetUserInfo(id);
	int *userInfo = new int[MAX_USER_SIZE];
	int cnt = 0;

	for (auto iter : mClients)
	{
		//if (iter->GetUserId() == id) continue;
		if (iter->GetChannelIndex() == user->GetChannelIndex())		// 채널이 같고
		{
			if (iter->GetRoomIndex() == user->GetRoomIndex())		// 방이 같아야 한다.
			{
				userInfo[cnt++] = iter->GetUserId();
			}
		}
	}
	SendRoomUserListPacket(id, message.roomindex(), userInfo, cnt);
}

int ChattingServer::WsaRecv(int id)
{
	User *userInfo = GetUserInfo(id);
	DWORD flags = { 0 };
	return WSARecv(userInfo->GetUserSocket(), &recv_over.wsabuf, 1, NULL, &flags, &recv_over.overlap, NULL);
}

int ChattingServer::SendPacket(int id, unsigned char * packet) const
{	
	User *userInfo = GetUserInfo(id);
	
	Overlap *over = new Overlap;
	memset(over, 0, sizeof(Overlap));
	over->event_type = OV_SEND;
	over->wsabuf.buf = reinterpret_cast<CHAR *>(over->iocp_buff);
	over->wsabuf.len = packet[4];
	memcpy(over->iocp_buff, packet, packet[4]);

	int ret = WSASend(userInfo->GetUserSocket(), &over->wsabuf, 1, NULL, 0,
		&over->overlap, NULL);
	return ret;
}

void ChattingServer::SendNotifyExistRoomPacket(google::protobuf::int32 id, google::protobuf::int32 room, bool exist) const
{
	Protocols::Notify_Exist_Room exist_room;
	exist_room.set_id(id);
	exist_room.set_roomindex(room);
	exist_room.set_exist(exist);

	size_t bufSize = exist_room.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::NOTIFY_EXIST_ROOM;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = exist_room.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	unsigned char* resultBuf = new unsigned char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);
	SendPacket(id, resultBuf);
}

void ChattingServer::SendRoomChattingPacket(int id, int target, std::string msg, int len) const
{
	Protocols::Room_Chatting chatting;
	chatting.set_id(id);
	chatting.set_message(msg);

	size_t bufSize = chatting.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::ROOM_CHATTING;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = chatting.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	unsigned char* resultBuf = new unsigned char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);
	SendPacket(target, resultBuf);
}

void ChattingServer::SendEnterRoomPacket(int id, bool enter, int room) const
{
	Protocols::Enter_Room enter_room;
	enter_room.set_id(id);
	enter_room.set_roomindex(room);
	enter_room.set_isenter(enter);

	size_t bufSize = enter_room.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::ENTER_ROOM;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = enter_room.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	unsigned char* resultBuf = new unsigned char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);
	SendPacket(id, resultBuf);
}

void ChattingServer::SendChannelChattingPacket(int id, int target, std::string msg, int len) const
{
	Protocols::Channel_Chatting chatting;
	chatting.set_id(id);
	chatting.set_message(msg);

	size_t bufSize = chatting.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::CHANNEL_CHATTING;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = chatting.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	unsigned char* resultBuf = new unsigned char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);
	SendPacket(target, resultBuf);
}

void ChattingServer::SendNotifyEnterRoomPacket(int id, int target) const
{
	Protocols::Notify_Enter_Room enter_room;
	enter_room.set_id(id);

	size_t bufSize = enter_room.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::NOTIFY_ENTER_ROOM;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = enter_room.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	unsigned char* resultBuf = new unsigned char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);
	SendPacket(target, resultBuf);
}

void ChattingServer::SendNotifyLeaveRoomPacket(int id, int target) const
{
	Protocols::Notify_Leave_Room leave_room;
	leave_room.set_id(id);

	size_t bufSize = leave_room.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::NOTIFY_LEAVE_ROOM;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = leave_room.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	unsigned char* resultBuf = new unsigned char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);
	SendPacket(target, resultBuf);
}

void ChattingServer::SendRoomUserListPacket(int id, int room, int* user, int userCnt) const
{
	Protocols::Room_List room_list;
	room_list.set_id(id);
	room_list.set_roomindex(room);
	room_list.set_usercount(userCnt);
	room_list.clear_userlist();
	for (int i = 0; i < userCnt; ++i)
	{
		room_list.add_userlist(user[i]);
	}

	size_t bufSize = room_list.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::ROOM_LIST;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = room_list.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	unsigned char* resultBuf = new unsigned char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);
	SendPacket(id, resultBuf);
}
