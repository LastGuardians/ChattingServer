#include "stdafx.h"

IMPLEMENT_SINGLETON(ChattingServer);

ChattingServer::ChattingServer()
{
	_recv_over.wsabuf.buf = reinterpret_cast<CHAR*>(_recv_over.iocp_buff);
	_recv_over.wsabuf.len = sizeof(_recv_over.iocp_buff);	
}

ChattingServer::~ChattingServer()
{
}


User* ChattingServer::GetUserInfo(int id) const
{
	auto iter = _mClients.find(id);
	if (iter != _mClients.end())
	{
		return iter->second;
	}
	return nullptr;
}

// 서버 초기화
bool ChattingServer::InitServer()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);
	if (NULL == _hiocp) 
	{ 
		err_display("InitServer() : ", WSAGetLastError()); 
		return false;
	}
	std::cout << "InitServer()" << std::endl;

	for (int i = 0; i < 5; ++i)
	{
		_channel.push_back(new Channel(i));
	}

	return true;
}

void ChattingServer::ServerThreadStart()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// accept 스레드 생성
	std::thread acceptThread{ &ChattingServer::AcceptThread , this };

	// worker 스레드 생성
	std::vector<std::thread *> workerThreads;
	for (DWORD i = 0; i < si.dwNumberOfProcessors; ++i)
	{
		workerThreads.push_back(new std::thread{ &ChattingServer::WorkerThread, this });
	}

	for (auto thread : workerThreads) {
		thread->join();
		delete thread;
	}

	acceptThread.join();
}

void ChattingServer::CloseSocket(unsigned long id)
{
	User *userInfo = GetUserInfo(id);
	if (nullptr == userInfo)
	{
		std::cout << "CloseSocket() : 해당 유저가 없습니다." << std::endl;
		return;
	}

	_channel[userInfo->GetChannelIndex()]->DeleteUserToChannel(userInfo);
	_channel[userInfo->GetChannelIndex()]->DeleteUserToRoom(userInfo->GetRoomIndex(), userInfo);

	std::cout << "[ " << id << " ] User CloseSocket!" << std::endl;
	
	_mClients.erase(id);
	closesocket(userInfo->GetUserSocket());
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
	_cs_lock.lock();
	printf("%s\n", msg);
	printf("err_no : %d\n", err_no);
	LocalFree(lpMsgBuf);
	_cs_lock.unlock();
}

void ChattingServer::AcceptThread()
{
	int retval;

	//socket()
	SOCKET listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
									NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == listen_sock) { err_display("WSASocket() : ", WSAGetLastError()); };
	std::cout << "socket()" << std::endl;

	//bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = ::bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_display("bind() : ", WSAGetLastError());
	std::cout << "bind()" << std::endl;

	//listen() 
	retval = ::listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_display("listen() : ", WSAGetLastError());
	std::cout << "listen()" << std::endl;

	SOCKADDR_IN clientaddr;
	int addrlen;
	srand(time(NULL));
	
	while (1)
	{
		//accept()
		addrlen = sizeof(clientaddr);
		std::cout << "Accept() Wait...." << std::endl;
		SOCKET client_sock = WSAAccept(listen_sock, reinterpret_cast<sockaddr *>(&clientaddr), &addrlen, NULL, NULL);
		if (client_sock == INVALID_SOCKET) {
			err_display("WSAAccept() : ", WSAGetLastError());
			break;
		}

		printf("\n[Server] Client Connect : IP = %s, Port = %d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		_clientId += 1;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_sock), _hiocp, _clientId, 0);
				
		User* userInfo = new User(client_sock, true, _clientId);
		_mClients[_clientId] = userInfo;

		// 접속한 클라이언트에게 랜덤한 채널 인덱스 부여 = 접속과 동시에 채널 입장
		Protocols::Enter_Channel enter;
		enter.set_id(_clientId);
		enter.set_channelindex(rand() % 5);
		enter.set_type(Protocols::ENTER_CHANNEL);

		userInfo->SetChannelIndex(enter.channelindex());
		userInfo->SetRoomIndex(-1);

		// 채널에 유저 입장
		_channel[enter.channelindex()]->AddUserToChannel(userInfo);

		int result = SendPacketAssemble(_clientId, enter.type(), dynamic_cast<google::protobuf::Message&>(enter));
		if (SOCKET_ERROR == result) {
			if (ERROR_IO_PENDING != WSAGetLastError()) {
				err_display("Accept::WSASend Error! : ", WSAGetLastError());
			}
		} 
				
		DWORD flags = { 0 };
		retval = WSARecv(userInfo->GetUserSocket(), &_recv_over.wsabuf, 1, NULL, &flags, &_recv_over.overlap, NULL);
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
	while (false == _server_shut_down)
	{
		unsigned long id = { 0 };
		DWORD io_size = { 0 };
		Overlap *ovlp = { nullptr };

		BOOL result = GetQueuedCompletionStatus(_hiocp, &io_size, (PULONG_PTR)&id,
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
			protobuf::io::ArrayInputStream input_array_stream(_recv_over.iocp_buff, BUFSIZE);
			protobuf::io::CodedInputStream input_coded_stream(&input_array_stream);

			// 패킷 분석
			PacketProcess(id, input_coded_stream);

			DWORD flags = { 0 };
			int retval = WSARecv(_mClients[id]->GetUserSocket(), &_recv_over.wsabuf, 1, NULL, &flags, &_recv_over.overlap, NULL);
			if (SOCKET_ERROR == retval) {
				int err_no = WSAGetLastError();
				if (ERROR_IO_PENDING != err_no) {
					err_display("WorkerThread::WSARecv", err_no);
					exit(-1);
				}
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

void ChattingServer::ProcessCreateRoomPacket(int id, const Protocols::Create_Room& message) const
{
	User *userInfo = GetUserInfo(id);

	Protocols::Notify_Exist_Room exist_room;
	exist_room.set_id(id);
	exist_room.set_roomindex(message.roomindex());
	exist_room.set_type(Protocols::NOTIFY_EXIST_ROOM);

	// 방이 하나도 없을 때는 바로 생성한다.
	if (true == _channel[userInfo->GetChannelIndex()]->GetRoomIsEmpty())
	{		
		exist_room.set_exist(false);
		SendPacketAssemble(id, exist_room.type(), exist_room);
		//SendNotifyExistRoomPacket(id, message.roomindex(), false);

		_channel[userInfo->GetChannelIndex()]->AddNewRoom(message.roomindex(), userInfo->GetChannelIndex());  // 채널에 방 추가
		_channel[userInfo->GetChannelIndex()]->AddUserToRoom(message.roomindex(), userInfo);	// 방에 유저 추가
		userInfo->SetRoomIndex(message.roomindex());

		std::cout << "[" << id << "] 유저가 " << "[" << message.roomindex()
			<< "] 번 방을 생성하였습니다." << std::endl;
	}

	// 방이 하나 이상일 때
	else
	{
		// 이미 존재하는 방인지 체크한다. 이미 존재하면 방 생성을 하지 않는다.
		if (true == _channel[userInfo->GetChannelIndex()]->GetRoomIsExist(message.roomindex()))
		{
			std::cout << message.roomindex() << "번 방이 이미 있습니다." << std::endl;

			exist_room.set_exist(true);
			SendPacketAssemble(id, exist_room.type(), exist_room);
			//SendNotifyExistRoomPacket(id, message.roomindex(), true);
			return;
		}

		// 존재하는 방이 없을 때
		_channel[userInfo->GetChannelIndex()]->AddNewRoom(message.roomindex(), userInfo->GetChannelIndex()); // 채널에 방 추가
		_channel[userInfo->GetChannelIndex()]->AddUserToRoom(message.roomindex(), userInfo);	// 방에 유저 추가
		userInfo->SetRoomIndex(message.roomindex());

		exist_room.set_exist(false);
		SendPacketAssemble(id, exist_room.type(), exist_room);
		//SendNotifyExistRoomPacket(id, message.roomindex(), false);

		std::cout << "[" << id << "] 유저가 " << "[" << message.roomindex()
			<< "] 번 방을 생성하였습니다." << std::endl;
	}
}

void ChattingServer::ProcessChangeChannelPacket(int id, const Protocols::Change_Channel& message) const
{
	User *userInfo = GetUserInfo(id);
	int currentChannelIndex = userInfo->GetChannelIndex();
	
	_channel[currentChannelIndex]->DeleteUserToChannel(userInfo);
	_channel[message.channelindex()]->AddUserToChannel(userInfo);

	userInfo->SetChannelIndex(message.channelindex());

	std::cout << "[" << id << "] 유저가 " << "[" << message.channelindex()
		<< "] 번 채널로 이동했습니다." << std::endl;
}

void ChattingServer::ProcessChannelChattingPacket(int id, const Protocols::Channel_Chatting& message) const
{
	User *userInfo = GetUserInfo(id);
	for (auto iter : _mClients)
	{
		_cs_lock.lock();
		//if (iter->GetUserId() == id) continue;
		if ((iter.second)->GetChannelIndex() == userInfo->GetChannelIndex())		// 채널이 같으면.
		{
			Protocols::Channel_Chatting chatting;
			chatting.set_id(id);
			chatting.set_message(message.message());
			chatting.set_type(Protocols::CHANNEL_CHATTING);

			SendPacketAssemble((iter.second)->GetUserId(), chatting.type(), chatting);
			//SendChannelChattingPacket(id, (iter.second)->GetUserId(), message.message(), message.ByteSize());
		}
		_cs_lock.unlock();
	}
}

void ChattingServer::ProcessRoomChattingPacket(int id, const Protocols::Room_Chatting& message) const
{
	User *userInfo = GetUserInfo(id);
	for (auto iter : _mClients)
	{
		_cs_lock.lock();
		//if(iter->GetUserId() == id) continue;									// 내 자신은 건너뛴다.
		if ((iter.second)->GetChannelIndex() == userInfo->GetChannelIndex())	// 채널이 같고
		{
			if ((iter.second)->GetRoomIndex() == userInfo->GetRoomIndex())		// 방이 같아야 메시지를 보낸다.
			{
				Protocols::Room_Chatting room;
				room.set_id(id);
				room.set_message(message.message());
				room.set_type(Protocols::ROOM_CHATTING);

				SendPacketAssemble((iter.second)->GetUserId(), room.type(), room);
				//SendRoomChattingPacket(id, (iter.second)->GetUserId(), message.message(), message.ByteSize());
			}
		}
		_cs_lock.unlock();
	}
}

void ChattingServer::ProcessEnterRoomPacket(int id, const Protocols::Enter_Room& message) const
{
	User *userInfo = GetUserInfo(id);

	Protocols::Enter_Room enter_room;
	enter_room.set_id(id);
	enter_room.set_roomindex(message.roomindex());
	enter_room.set_type(Protocols::ENTER_ROOM);

	// 방이 존재한다면 바로 입장.
	if (true == _channel[userInfo->GetChannelIndex()]->GetRoomIsExist(message.roomindex()))
	{
		std::cout << "[" << id << "] 유저가 " << "[" << message.roomindex()
			<< "] 번 방에 입장하였습니다." << std::endl;

		_channel[userInfo->GetChannelIndex()]->AddUserToRoom(message.roomindex(), userInfo);
		userInfo->SetRoomIndex(message.roomindex());

		// 방 입장 성공
		enter_room.set_isenter(true);
		SendPacketAssemble(id, enter_room.type(), enter_room);
		//SendEnterRoomPacket(id, true, message.roomindex());

		_cs_lock.lock();
		for (auto iter : _mClients)
		{			
			if ((iter.second)->GetUserId() == id) continue;
			if ((iter.second)->GetChannelIndex() == userInfo->GetChannelIndex())		// 채널이 같고
			{
				if ((iter.second)->GetRoomIndex() == userInfo->GetRoomIndex())			// 방이 같아야 한다.
				{
					Protocols::Notify_Enter_Room noti_room;
					noti_room.set_id(id);
					noti_room.set_type(Protocols::NOTIFY_ENTER_ROOM);

					SendPacketAssemble((iter.second)->GetUserId(), noti_room.type(), noti_room);
					//SendNotifyEnterRoomPacket(id, (iter.second)->GetUserId());			// 방 입장 패킷을 보낸다.
				}
			}			
		}
		_cs_lock.unlock();
		return;
	}

	// 방 입장 실패 
	std::cout << message.roomindex() << " 번 방이 없습니다." << std::endl;

	enter_room.set_isenter(false);
	SendPacketAssemble(id, enter_room.type(), enter_room);
	//SendEnterRoomPacket(id, false, message.roomindex());
}

void ChattingServer::ProcessLeaveRoomPacket(int id, const Protocols::Leave_Room& message) const
{
	User *userInfo = GetUserInfo(id);
	_channel[userInfo->GetChannelIndex()]->DeleteUserToRoom(message.roomindex(), userInfo);
	userInfo->SetRoomIndex(0);		// 유저 방 초기화

	_cs_lock.lock();
	for (auto iter : _mClients)
	{		
		if ((iter.second)->GetUserId() == id) continue;
		if ((iter.second)->GetChannelIndex() == userInfo->GetChannelIndex())		// 채널이 같고
		{
			if ((iter.second)->GetRoomIndex() == message.roomindex())				// 방이 같아야 한다.
			{
				Protocols::Notify_Leave_Room leave_room;
				leave_room.set_id(id);
				leave_room.set_type(Protocols::NOTIFY_LEAVE_ROOM);

				SendPacketAssemble((iter.second)->GetUserId(), leave_room.type(), leave_room);
				//SendNotifyLeaveRoomPacket(id, (iter.second)->GetUserId());			// 방 퇴장 패킷을 보낸다.
			}
		}		
	}
	_cs_lock.unlock();
}

void ChattingServer::ProcessRoomUserListPacket(int id, const Protocols::Room_List& message) const
{
	User *user = GetUserInfo(id);
	int userInfo[MAX_USER_SIZE];
	int cnt = 0;

	for (auto iter : _mClients)
	{
		_cs_lock.lock();
		//if (iter->GetUserId() == id) continue;
		if ((iter.second)->GetChannelIndex() == user->GetChannelIndex())		// 채널이 같고
		{
			if ((iter.second)->GetRoomIndex() == user->GetRoomIndex())		// 방이 같아야 한다.
			{
				userInfo[cnt++] = (iter.second)->GetUserId();
			}
		}
		_cs_lock.unlock();
	}

	Protocols::Room_List room_list;
	room_list.set_id(id);
	room_list.set_roomindex(message.roomindex());
	room_list.set_usercount(cnt);
	room_list.clear_userlist();
	for (int i = 0; i < cnt; ++i)
	{
		room_list.add_userlist(userInfo[i]);
	}
	room_list.set_type(Protocols::ROOM_LIST);

	SendPacketAssemble(id, room_list.type(), room_list);
	//SendRoomUserListPacket(id, message.roomindex(), userInfo, cnt);
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

// sendPacket 중복되는 기능 합침 -> msg 에서 type을 바로 가져올 순 없을까?
int ChattingServer::SendPacketAssemble(int id, int type, google::protobuf::Message & msg) const
{
	size_t bufSize = msg.ByteSizeLong();
	unsigned char resultBuf[100];
	int size = bufSize + MessageHeaderSize;
	
	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = type;

	char* header_seri = reinterpret_cast<char*>(&header);

	memcpy(resultBuf, header_seri, MessageHeaderSize);
	msg.SerializeToArray(resultBuf + MessageHeaderSize, bufSize);
	
	return SendPacket(id, resultBuf);
}
