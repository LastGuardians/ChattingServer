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


User* ChattingServer::GetUserInfo(int clientId)
{
	for (auto iter : mClients)
	{
		if (iter->GetUserId() == clientId)
		{
			return iter;
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
	Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].DeleteUserToChannel(mClients[id]);
	Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].DeleteUserToRoom(mClients[id]->GetRoomIndex(), mClients[id]);
	mClients[id]->SetChannelIndex(-1);
	mClients[id]->SetRoomIndex(-1);
	std::cout << "[ " <<id << " ] User CloseSocket!" << std::endl;
	
	closesocket(mClients[id]->GetUserSocket());
	//mClients[id]-> = false;
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

		// ���� �� ���� : 50��
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

		/*Enter_Channel enter_channel_packet;
		enter_channel_packet.size = sizeof(enter_channel_packet);
		enter_channel_packet.id = clientId;
		enter_channel_packet.channelIndex = rand() % 5;*/
		mClients[clientId]->SetChannelIndex(enter.channelindex());
		mClients[clientId]->SetRoomIndex(-1);

		// ä�ο� ���� ����		
		Singleton::GetInstance()->channel[enter.channelindex()].AddUserToChannel(mClients[clientId]);

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
			ChattingServer handler;
			protobuf::io::ArrayInputStream input_array_stream(recv_over.iocp_buff, BUFSIZE);
			protobuf::io::CodedInputStream input_coded_stream(&input_array_stream);

			// ��Ŷ �м�
			PacketProcess(input_coded_stream, handler);
			int retval = WsaRecv(id);

			//int retval = PacketRessembly(id, io_size);
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

int ChattingServer::PacketRessembly(int id, DWORD packetSize)
{
	unsigned char *buf_ptr = recv_over.iocp_buff;
	ProcessPacket(id, buf_ptr);

	return WsaRecv(id);
}

void ChattingServer::ProcessPacket(int id, unsigned char * buf)
{
	switch (buf[0])
	{
	case ENTER_CHANNEL:
		break;
	case CREATE_ROOM:
		ProcessCreateRoomPacket(id, buf);
		break;
	case CHANGE_CHANNEL:
		ProcessChangeChannelPacket(id, buf);
		break;
	case ROOM_CHATTING:
		ProcessRoomChattingPacket(id, buf);
		break;
	case ENTER_ROOM:
		ProcessEnterRoomPacket(id, buf);
		break;
	case ROOM_LIST:
		ProcessRoomUserListPacket(id, buf);
		break;
	case CHANNEL_CHATTING:
		ProcessChannelChattingPacket(id, buf);
		break;
	case LEAVE_ROOM:
		ProcessLeaveRoomPacket(id, buf);
		break;
	default:
		break;
	}
}

void ChattingServer::PacketProcess(protobuf::io::CodedInputStream & input_stream, const ChattingServer & handler)
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
			if (false == message.ParseFromCodedStream(&payload_input_stream))
				break;
			handler.ProcessCreateRoomPacket(message);

			break;
		}
		}
	}
}

void ChattingServer::ProcessCreateRoomPacket(const Protocols::Create_Room message) const
{
	int id = message.id();

	// ���� �ϳ��� ���� ���� �ٷ� �����Ѵ�.
	if (true == Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].GetRoomIsEmpty())
	{
		//Singleton::GetInstance()->roomList.emplace_back(packet->roomIndex);

		SendNotifyExistRoomPacket(id, message.roomindex(), false);

		Room *newRoom = new Room(message.roomindex(), mClients[id]->GetChannelIndex());	// ���ο� �� ����
		Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].AddNewRoom(message.roomindex(), newRoom);  // ä�ο� �� �߰�
		Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].AddUserToRoom(message.roomindex(), mClients[id]);	// �濡 ���� �߰�
		mClients[id]->SetRoomIndex(message.roomindex());

		std::cout << "[" << message.id() << "] ������ " << "[" << message.roomindex()
			<< "] �� ���� �����Ͽ����ϴ�." << std::endl;
	}

	// ���� �ϳ� �̻��� ��
	else
	{
		// �̹� �����ϴ� ������ üũ�Ѵ�. �̹� �����ϸ� �� ������ ���� �ʴ´�.
		if (true == Singleton::GetInstance()->channel[mClients[message.id()]->GetChannelIndex()].GetRoomIsExist(message.roomindex()))
		{
			std::cout << message.roomindex() << "�� ���� �̹� �ֽ��ϴ�." << std::endl;

			SendNotifyExistRoomPacket(message.id(), message.roomindex(), true);
			return;
		}

		// �����ϴ� ���� ���� ��
		Room *newRoom = new Room(message.roomindex(), mClients[id]->GetChannelIndex());	// ���ο� �� ����
		Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].AddNewRoom(message.roomindex(), newRoom); // ä�ο� �� �߰�
		Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].AddUserToRoom(message.roomindex(), mClients[id]);	// �濡 ���� �߰�
		mClients[id]->SetRoomIndex(message.roomindex());

		SendNotifyExistRoomPacket(id, message.roomindex(), false);

		std::cout << "[" << message.id() << "] ������ " << "[" << message.roomindex()
			<< "] �� ���� �����Ͽ����ϴ�." << std::endl;
	}
}


void ChattingServer::ProcessCreateRoomPacket(int id, unsigned char * buf)
{
	Create_Room *packet = reinterpret_cast<Create_Room*>(buf);

	// ���ο� �� ���� (����-> ������ �濡 ������ ���?)
	// ��� �̱��� ������ �����ϴ� ���� ���� �ȵ��. -> ���? ä�θŴ���?

	// ���� �ϳ��� ���� ���� �ٷ� �����Ѵ�.
	if (true == Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].GetRoomIsEmpty())
	{
		//Singleton::GetInstance()->roomList.emplace_back(packet->roomIndex);

		SendNotifyExistRoomPacket(id, packet->roomIndex, false);

		Room *newRoom = new Room(packet->roomIndex, mClients[id]->GetChannelIndex());	// ���ο� �� ����
		Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].AddNewRoom(packet->roomIndex, newRoom);  // ä�ο� �� �߰�
		Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].AddUserToRoom(packet->roomIndex, mClients[id]);	// �濡 ���� �߰�
		mClients[id]->SetRoomIndex(packet->roomIndex);

		std::cout << "[" << id << "] ������ " << "[" << packet->roomIndex
			<< "] �� ���� �����Ͽ����ϴ�." << std::endl;
	}

	// ���� �ϳ� �̻��� ��
	else 
	{
		// �̹� �����ϴ� ������ üũ�Ѵ�. �̹� �����ϸ� �� ������ ���� �ʴ´�.
		if (true == Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].GetRoomIsExist(packet->roomIndex))
		{
			std::cout << packet->roomIndex << "�� ���� �̹� �ֽ��ϴ�." << std::endl;

			SendNotifyExistRoomPacket(id, packet->roomIndex, true);
			return;
		}
		/*for (auto iter = Singleton::GetInstance()->roomList.begin(); iter != Singleton::GetInstance()->roomList.end(); ++iter)
		{
			if ((*iter) == packet->roomIndex)
			{
				std::cout << packet->roomIndex << "�� ���� �̹� �ֽ��ϴ�." << std::endl;

				SendNotifyExistRoomPacket(id, packet->roomIndex, true);
				return;
			}
		}*/

		// �����ϴ� ���� ���� ��
		Room *newRoom = new Room(packet->roomIndex, mClients[id]->GetChannelIndex());	// ���ο� �� ����
		//Singleton::GetInstance()->roomList.emplace_back(packet->roomIndex);
		
		Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].AddNewRoom(packet->roomIndex, newRoom); // ä�ο� �� �߰�
		Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].AddUserToRoom(packet->roomIndex, mClients[id]);	// �濡 ���� �߰�
		mClients[id]->SetRoomIndex(packet->roomIndex);

		SendNotifyExistRoomPacket(id, packet->roomIndex, false);
		
		std::cout << "[" << id << "] ������ " << "[" << packet->roomIndex
			<< "] �� ���� �����Ͽ����ϴ�." << std::endl;
	}

}

void ChattingServer::ProcessChangeChannelPacket(int id, unsigned char * buf)
{
	int _currentChannelIndex = mClients[id]->GetChannelIndex();
	User* userInfo = mClients[id];

	Change_Channel *packet = reinterpret_cast<Change_Channel*>(buf);
	Singleton::GetInstance()->channel[_currentChannelIndex].DeleteUserToChannel(userInfo);
	Singleton::GetInstance()->channel[packet->channelIndex].AddUserToChannel(userInfo);

	userInfo->SetChannelIndex(packet->channelIndex);

	std::cout << "[" << id << "] ������ " << "[" << packet->channelIndex
		<< "] �� ä�η� �̵��߽��ϴ�." << std::endl;
}

void ChattingServer::ProcessRoomChattingPacket(int id, unsigned char * buf)
{	
	Room_Chatting *chat_packet = reinterpret_cast<Room_Chatting*>(buf);

	for (auto iter : mClients)
	{
		//if(iter->GetUserId() == id) continue;								// �� �ڽ��� �ǳʶڴ�.
		if (iter->GetChannelIndex() == mClients[id]->GetChannelIndex())		// ä���� ����
		{
			if (iter->GetRoomIndex() == mClients[id]->GetRoomIndex())		// ���� ���ƾ� �޽����� ������.
			{
				SendRoomChattingPacket(id, iter->GetUserId(), chat_packet->message, chat_packet->size);
			}
		}
	}		
	/*	Channel_P::channel_chatting* recv_msg = new Channel_P::channel_chatting();
	bool result = recv_msg->ParseFromArray(buf, 1000);
	if (false == result)
	printf("Deserialize Failed! \n");*/
}

// �� ���� ��Ŷ ó��
void ChattingServer::ProcessEnterRoomPacket(int id, unsigned char * buf)
{
	Enter_Room *enter_packet = reinterpret_cast<Enter_Room*>(buf);

	// ���� �����Ѵٸ� �ٷ� ����.
	if (true == Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].GetRoomIsExist(enter_packet->roomIndex))
	{
		std::cout << "[" << id << "] ������ " << "[" << enter_packet->roomIndex
			<< "] �� �濡 �����Ͽ����ϴ�." << std::endl;

		Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].AddUserToRoom(enter_packet->roomIndex, mClients[id]);
		mClients[id]->SetRoomIndex(enter_packet->roomIndex);

		// �� ���� ����
		SendEnterRoomPacket(id, true, enter_packet->roomIndex);

		for (auto iter : mClients)
		{
			if (iter->GetUserId() == id) continue;
			if (iter->GetChannelIndex() == mClients[id]->GetChannelIndex())		// ä���� ����
			{
				if (iter->GetRoomIndex() == mClients[id]->GetRoomIndex())		// ���� ���ƾ� �Ѵ�.
				{
					SendNotifyEnterRoomPacket(id, iter->GetUserId());			// �� ���� ��Ŷ�� ������.
				}
			}
		}
		return;
	}

	//for (auto iter = Singleton::GetInstance()->roomList.begin(); iter != Singleton::GetInstance()->roomList.end(); ++iter)
	//{
	//	if ((*iter) == enter_packet->roomIndex)
	//	{
	//		std::cout << "[" << id << "] ������ " << "[" << enter_packet->roomIndex
	//			<< "] �� �濡 �����Ͽ����ϴ�." << std::endl;

	//		Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].AddUserToRoom(enter_packet->roomIndex, mClients[id]);
	//		mClients[id]->SetRoomIndex(enter_packet->roomIndex);

	//		// �� ���� ����
	//		SendEnterRoomPacket(id, true, enter_packet->roomIndex);

	//		for (auto iter : mClients)
	//		{
	//			if (iter->GetUserId() == id) continue;
	//			if (iter->GetChannelIndex() == mClients[id]->GetChannelIndex())		// ä���� ����
	//			{
	//				if (iter->GetRoomIndex() == mClients[id]->GetRoomIndex())		// ���� ���ƾ� �Ѵ�.
	//				{
	//					SendNotifyEnterRoomPacket(id, iter->GetUserId());			// �� ���� ��Ŷ�� ������.
	//				}
	//			}
	//		}
	//		return;
	//	}
	//}

	// �� ���� ���� 
	std::cout << enter_packet->roomIndex << " �� ���� �����ϴ�." << std::endl;
	SendEnterRoomPacket(id, false, enter_packet->roomIndex);
}

void ChattingServer::ProcessChannelChattingPacket(int id, unsigned char * buf) const
{
	Channel_Chatting *chat_packet = reinterpret_cast<Channel_Chatting*>(buf);

	for (auto iter : mClients)
	{
		if (iter->GetUserId() == id) continue;
		if (iter->GetChannelIndex() == mClients[id]->GetChannelIndex())		// ä���� ������.
		{			
			SendChannelChattingPacket(id, iter->GetUserId(), chat_packet->message, chat_packet->size);
		}
	}

}

void ChattingServer::ProcessLeaveRoomPacket(int id, unsigned char * buf)
{
	Leave_Room *leave_packet = reinterpret_cast<Leave_Room*>(buf);
	Singleton::GetInstance()->channel[mClients[id]->GetChannelIndex()].DeleteUserToRoom(leave_packet->roomIndex, mClients[id]);
	mClients[id]->SetRoomIndex(0);		// ���� �� �ʱ�ȭ

	for (auto iter : mClients)
	{
		if (iter->GetUserId() == id) continue;
		if (iter->GetChannelIndex() == mClients[id]->GetChannelIndex())		// ä���� ����
		{
			if (iter->GetRoomIndex() == leave_packet->roomIndex)		// ���� ���ƾ� �Ѵ�.
			{
				SendNotifyLeaveRoomPacket(id, iter->GetUserId());			// �� ���� ��Ŷ�� ������.
			}
		}
	}
}

void ChattingServer::ProcessRoomUserListPacket(int id, unsigned char * buf)
{
	Room_List *room_packet = reinterpret_cast<Room_List*>(buf);

	int *userInfo = new int[MAX_USER_SIZE];
	int cnt = 0;

	for (auto iter : mClients)
	{
		//if (iter->GetUserId() == id) continue;
		if (iter->GetChannelIndex() == mClients[id]->GetChannelIndex())		// ä���� ����
		{
			if (iter->GetRoomIndex() == mClients[id]->GetRoomIndex())		// ���� ���ƾ� �Ѵ�.
			{
				userInfo[cnt++] = iter->GetUserId();
			}
		}
	}
	SendRoomUserListPacket(id, room_packet->roomIndex, userInfo, cnt);
}

int ChattingServer::WsaRecv(int id)
{
	DWORD flags = { 0 };
	return WSARecv(mClients[id]->GetUserSocket(), &recv_over.wsabuf, 1, NULL, &flags, &recv_over.overlap, NULL);
}

int ChattingServer::SendPacket(int id, unsigned char * packet) const
{
	Overlap *over = new Overlap;
	memset(over, 0, sizeof(Overlap));
	over->event_type = OV_SEND;
	over->wsabuf.buf = reinterpret_cast<CHAR *>(over->iocp_buff);
	over->wsabuf.len = packet[4];
	memcpy(over->iocp_buff, packet, packet[4]);

	int ret = WSASend(mClients[id]->GetUserSocket(), &over->wsabuf, 1, NULL, 0,
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

	//Notify_Exist_Room exist_packet;
	//exist_packet.size = sizeof(Notify_Exist_Room);
	//exist_packet.type = NOTIFY_EXIST_ROOM;
	//exist_packet.id = id;
	//exist_packet.roomIndex = room;
	//exist_packet.exist = exist;
	SendPacket(id, resultBuf);
}

void ChattingServer::SendRoomChattingPacket(int id, int target, char * msg, int len)
{
	Room_Chatting chat_packet;
	chat_packet.id = id;
	chat_packet.type = ROOM_CHATTING;
	chat_packet.roomIndex = mClients[target]->GetRoomIndex();
	//strncpy(chat_packet->message, msg, len);
	strcpy(chat_packet.message, msg);
	chat_packet.size = len;
	SendPacket(target, reinterpret_cast<unsigned char*>(&chat_packet));
}

void ChattingServer::SendEnterRoomPacket(int id, bool enter, int room)
{
	Enter_Room enter_packet;
	enter_packet.id = id;
	enter_packet.type = ENTER_ROOM;
	enter_packet.size = sizeof(Enter_Room);
	enter_packet.isEnter = enter;
	enter_packet.roomIndex = room;
	SendPacket(id, reinterpret_cast<unsigned char*>(&enter_packet));
}

void ChattingServer::SendChannelChattingPacket(int id, int target, char * msg, int len) const
{
	Channel_Chatting chat_packet;
	chat_packet.id = id;
	chat_packet.type = CHANNEL_CHATTING;
	chat_packet.channelIndex = mClients[target]->GetChannelIndex();
 	strcpy(chat_packet.message, msg);
	chat_packet.size = len;
	SendPacket(target, reinterpret_cast<unsigned char*>(&chat_packet));
}

void ChattingServer::SendNotifyEnterRoomPacket(int id, int target)
{
	Notify_Enter_Room enter_packet;
	enter_packet.id = id;
	enter_packet.type = NOTIFY_ENTER_ROOM;
	enter_packet.size = sizeof(Notify_Enter_Room);
	SendPacket(target, reinterpret_cast<unsigned char*>(&enter_packet));
}

void ChattingServer::SendNotifyLeaveRoomPacket(int id, int target)
{
	Notify_Leave_Room leave_packet;
	leave_packet.id = id;
	leave_packet.type = NOTIFY_LEAVE_ROOM;
	leave_packet.size = sizeof(Notify_Leave_Room);
	SendPacket(target, reinterpret_cast<unsigned char*>(&leave_packet));
}

void ChattingServer::SendRoomUserListPacket(int id, int room, int* user, int userCnt)
{
	Room_List room_packet;
	room_packet.id = id;
	room_packet.type = ROOM_LIST;
	room_packet.size = sizeof(Room_List);
	room_packet.roomIndex = room;
	room_packet.userCount = userCnt;
	memcpy(room_packet.userList, user, MAX_USER_SIZE);
	//*room_packet.userList = *user;
	SendPacket(id, reinterpret_cast<unsigned char*>(&room_packet));
}
