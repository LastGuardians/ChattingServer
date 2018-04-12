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

// ���� �ʱ�ȭ
void ChattingServer::InitServer()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	m_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);
	if (NULL == m_hiocp) { err_display("InitServer() : ", WSAGetLastError()); }

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int m_cpu_core = static_cast<int>(si.dwNumberOfProcessors) / 2;

	// accept ������ ����
	std::thread acceptThread{ &ChattingServer::AcceptThread , this };

	// worker ������ ����
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
		// ������ Ŭ���̾�Ʈ���� ������ ä�� �ε��� �ο� = ���Ӱ� ���ÿ� ä�� ����
		Protocols::Enter_Channel enter;
		enter.set_id(clientId);
		enter.set_channelindex(rand() % 5);

		size_t bufSize = enter.ByteSizeLong();
		char* outputBuf = new char[bufSize];

		// ��� ����
		MessageHeader header;
		header.size = MessageHeaderSize + bufSize;
		header.type = Protocols::ENTER_CHANNEL;
		char* header_seri = reinterpret_cast<char*>(&header);

		int rtn = enter.SerializeToArray(outputBuf, bufSize);

		// ���� ���� ����
		unsigned char* resultBuf = new unsigned char[bufSize + MessageHeaderSize];
		memcpy(resultBuf, header_seri, MessageHeaderSize);
		memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);

		userInfo->SetChannelIndex(enter.channelindex());
		userInfo->SetRoomIndex(-1);

		// ä�ο� ���� ����		
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
			// ���� ������ Ŭ�� disconnect
			CloseSocket(id);
			continue;
		}
		if (OV_RECV == ovlp->event_type) {
			protobuf::io::ArrayInputStream input_array_stream(recv_over.iocp_buff, BUFSIZE);
			protobuf::io::CodedInputStream input_coded_stream(&input_array_stream);

			// ��Ŷ �м�
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
	
	// ����� �о
	while (input_stream.ReadRaw(&messageHeader, MessageHeaderSize))
	{
		// ���� �＼�� �Ҽ� �ִ� ���� �����Ϳ� ���� ���̸� �˾Ƴ�
		const void* payload_ptr = NULL;
		int remainSize = 0;
		input_stream.GetDirectBufferPointer(&payload_ptr, &remainSize);
		if (remainSize < (signed)messageHeader.size)
			break;

		// �޼��� ��ü�� �о�� ���� ��Ʈ���� ����
		protobuf::io::ArrayInputStream payload_array_stream(payload_ptr, messageHeader.size);
		protobuf::io::CodedInputStream payload_input_stream(&payload_array_stream);

		// �޼��� ��ü ������ ��ŭ ������ ����
		input_stream.Skip(messageHeader.size);

		// �޼��� �������� ������ȭ�ؼ� ������ �޼��带 ȣ������
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

	// ���� �ϳ��� ���� ���� �ٷ� �����Ѵ�.
	if (true == Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].GetRoomIsEmpty())
	{
		SendNotifyExistRoomPacket(id, message.roomindex(), false);

		Room *newRoom = new Room(message.roomindex(), userInfo->GetChannelIndex());	// ���ο� �� ����
		Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].AddNewRoom(message.roomindex(), newRoom);  // ä�ο� �� �߰�
		Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].AddUserToRoom(message.roomindex(), userInfo);	// �濡 ���� �߰�
		userInfo->SetRoomIndex(message.roomindex());

		std::cout << "[" << id << "] ������ " << "[" << message.roomindex()
			<< "] �� ���� �����Ͽ����ϴ�." << std::endl;
	}

	// ���� �ϳ� �̻��� ��
	else
	{
		// �̹� �����ϴ� ������ üũ�Ѵ�. �̹� �����ϸ� �� ������ ���� �ʴ´�.
		if (true == Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].GetRoomIsExist(message.roomindex()))
		{
			std::cout << message.roomindex() << "�� ���� �̹� �ֽ��ϴ�." << std::endl;

			SendNotifyExistRoomPacket(id, message.roomindex(), true);
			return;
		}

		// �����ϴ� ���� ���� ��
		Room *newRoom = new Room(message.roomindex(), userInfo->GetChannelIndex());	// ���ο� �� ����
		Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].AddNewRoom(message.roomindex(), newRoom); // ä�ο� �� �߰�
		Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].AddUserToRoom(message.roomindex(), userInfo);	// �濡 ���� �߰�
		userInfo->SetRoomIndex(message.roomindex());

		SendNotifyExistRoomPacket(id, message.roomindex(), false);

		std::cout << "[" << id << "] ������ " << "[" << message.roomindex()
			<< "] �� ���� �����Ͽ����ϴ�." << std::endl;
	}
}

void ChattingServer::ProcessChangeChannelPacket(int id, const Protocols::Change_Channel message) const
{
	User *userInfo = GetUserInfo(id);
	int _currentChannelIndex = userInfo->GetChannelIndex();
	
	Singleton::GetInstance()->channel[_currentChannelIndex].DeleteUserToChannel(userInfo);
	Singleton::GetInstance()->channel[message.channelindex()].AddUserToChannel(userInfo);

	userInfo->SetChannelIndex(message.channelindex());

	std::cout << "[" << id << "] ������ " << "[" << message.channelindex()
		<< "] �� ä�η� �̵��߽��ϴ�." << std::endl;
}

void ChattingServer::ProcessChannelChattingPacket(int id, const Protocols::Channel_Chatting message) const
{
	User *userInfo = GetUserInfo(id);
	for (auto iter : mClients)
	{
		//if (iter->GetUserId() == id) continue;
		if (iter->GetChannelIndex() == userInfo->GetChannelIndex())		// ä���� ������.
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
		//if(iter->GetUserId() == id) continue;								// �� �ڽ��� �ǳʶڴ�.
		if (iter->GetChannelIndex() == userInfo->GetChannelIndex())		// ä���� ����
		{
			if (iter->GetRoomIndex() == userInfo->GetRoomIndex())		// ���� ���ƾ� �޽����� ������.
			{
				SendRoomChattingPacket(id, iter->GetUserId(), message.message(), message.ByteSize());
			}
		}
	}
}

void ChattingServer::ProcessEnterRoomPacket(int id, const Protocols::Enter_Room message) const
{
	User *userInfo = GetUserInfo(id);
	// ���� �����Ѵٸ� �ٷ� ����.
	if (true == Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].GetRoomIsExist(message.roomindex()))
	{
		std::cout << "[" << id << "] ������ " << "[" << message.roomindex()
			<< "] �� �濡 �����Ͽ����ϴ�." << std::endl;

		Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].AddUserToRoom(message.roomindex(), userInfo);
		userInfo->SetRoomIndex(message.roomindex());

		// �� ���� ����
		SendEnterRoomPacket(id, true, message.roomindex());

		for (auto iter : mClients)
		{
			if (iter->GetUserId() == id) continue;
			if (iter->GetChannelIndex() == userInfo->GetChannelIndex())		// ä���� ����
			{
				if (iter->GetRoomIndex() == userInfo->GetRoomIndex())		// ���� ���ƾ� �Ѵ�.
				{
					SendNotifyEnterRoomPacket(id, iter->GetUserId());			// �� ���� ��Ŷ�� ������.
				}
			}
		}
		return;
	}

	// �� ���� ���� 
	std::cout << message.roomindex() << " �� ���� �����ϴ�." << std::endl;
	SendEnterRoomPacket(id, false, message.roomindex());
}

void ChattingServer::ProcessLeaveRoomPacket(int id, const Protocols::Leave_Room message) const
{
	User *userInfo = GetUserInfo(id);
	Singleton::GetInstance()->channel[userInfo->GetChannelIndex()].DeleteUserToRoom(message.roomindex(), userInfo);
	userInfo->SetRoomIndex(0);		// ���� �� �ʱ�ȭ

	for (auto iter : mClients)
	{
		if (iter->GetUserId() == id) continue;
		if (iter->GetChannelIndex() == userInfo->GetChannelIndex())		// ä���� ����
		{
			if (iter->GetRoomIndex() == message.roomindex())				// ���� ���ƾ� �Ѵ�.
			{
				SendNotifyLeaveRoomPacket(id, iter->GetUserId());			// �� ���� ��Ŷ�� ������.
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
		if (iter->GetChannelIndex() == user->GetChannelIndex())		// ä���� ����
		{
			if (iter->GetRoomIndex() == user->GetRoomIndex())		// ���� ���ƾ� �Ѵ�.
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

	// ��� ����
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::NOTIFY_EXIST_ROOM;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = exist_room.SerializeToArray(outputBuf, bufSize);

	// ���� ���� ����
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

	// ��� ����
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::ROOM_CHATTING;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = chatting.SerializeToArray(outputBuf, bufSize);

	// ���� ���� ����
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

	// ��� ����
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::ENTER_ROOM;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = enter_room.SerializeToArray(outputBuf, bufSize);

	// ���� ���� ����
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

	// ��� ����
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::CHANNEL_CHATTING;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = chatting.SerializeToArray(outputBuf, bufSize);

	// ���� ���� ����
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

	// ��� ����
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::NOTIFY_ENTER_ROOM;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = enter_room.SerializeToArray(outputBuf, bufSize);

	// ���� ���� ����
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

	// ��� ����
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::NOTIFY_LEAVE_ROOM;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = leave_room.SerializeToArray(outputBuf, bufSize);

	// ���� ���� ����
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

	// ��� ����
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::ROOM_LIST;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = room_list.SerializeToArray(outputBuf, bufSize);

	// ���� ���� ����
	unsigned char* resultBuf = new unsigned char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);
	SendPacket(id, resultBuf);
}
