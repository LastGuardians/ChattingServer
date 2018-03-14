#include "stdafx.h"

User::User() : userSocket{0}, userId{-1}
{
	recv_over.wsabuf.buf = reinterpret_cast<CHAR*>(recv_over.iocp_buff);
	recv_over.wsabuf.len = sizeof(recv_over.iocp_buff);
}

User::User(int id)
{
	userId = id;
}

User::User(SOCKET s, bool connect, int id)
{

}


User::~User()
{

}

void User::SetUserInfo(SOCKET s, bool connect, int id)
{
	userSocket = s;
	userConnected = connect;
	userId = id;
}

void User::SetChannelIndex(int channel)
{
	channel_index = channel;
}

int User::SendPacket(unsigned char *packet)
{
	Overlap *over = new Overlap;
	memset(over, 0, sizeof(Overlap));
	over->event_type = OV_SEND;
	over->wsabuf.buf = reinterpret_cast<CHAR *>(over->iocp_buff);
	over->wsabuf.len = packet[1];
	memcpy(over->iocp_buff, packet, packet[1]);

	int ret = WSASend(userSocket, &over->wsabuf, 1, NULL, 0,
		&over->overlap, NULL);
	return ret;
}

void User::SendNotifyExistRoomPacket(int room, bool exist)
{
	Notify_Exist_Room exist_packet;
	exist_packet.size = sizeof(Notify_Exist_Room);
	exist_packet.type = NOTIFY_EXIST_ROOM;
	exist_packet.id = userId;
	exist_packet.roomIndex = room;
	exist_packet.exist = exist;
	SendPacket(reinterpret_cast<unsigned char*>(&exist_packet));
}

void User::SendRoomListPacket()
{
	Room_List list_packet;

	if (Singletone::GetInstance()->roomList.size() == 0)
	{
		list_packet.id = userId;
		list_packet.type = ROOM_LIST;
		list_packet.size = sizeof(Room_List);
		list_packet.roomCnt = 0;
		SendPacket(reinterpret_cast<unsigned char*>(&list_packet));
		return;
	}

	list_packet.size = sizeof(Room_List);

	int cnt = 0;
	for (auto iter = Singletone::GetInstance()->roomList.begin(); iter != Singletone::GetInstance()->roomList.end(); ++iter)
	{
		list_packet.roomList[cnt++] = *iter;
	}
	cnt = 0;

	list_packet.id = userId;
	list_packet.type = ROOM_LIST;
	list_packet.roomCnt = Singletone::GetInstance()->roomList.size();
	SendPacket(reinterpret_cast<unsigned char*>(&list_packet));

}

int User::WsaRecv()
{
	DWORD flags = { 0 };
	return WSARecv(userSocket, &recv_over.wsabuf, 1, NULL, &flags, &recv_over.overlap, NULL);

}

void User::CloseSocket()
{
	closesocket(userSocket);
	userConnected = false;
}

int User::PacketRessembly(int id, DWORD packetSize)
{
	unsigned char *buf_ptr = recv_over.iocp_buff;
	ProcessPacket(id, buf_ptr);

	/*int remained = packetSize;
	while (0 < remained) {
		if (0 == recv_buff.sizeCurr) { recv_buff.sizeCurr = buf_ptr[0]; }
		int required = recv_buff.sizeCurr - recv_buff.sizePrev;
		if (remained >= required) {
			memcpy(recv_buff.buf + recv_buff.sizePrev, buf_ptr, required);
			ProcessPacket(id, buf_ptr);
			buf_ptr += required;
			remained -= required;
			recv_buff.sizeCurr = 0;
			recv_buff.sizePrev = 0;
		}
		else {
			memcpy(recv_buff.buf + recv_buff.sizePrev, buf_ptr, remained);
			buf_ptr += remained;
			recv_buff.sizePrev += remained;
			remained = 0;
		}
	}*/

	return WsaRecv();
}

void User::ProcessPacket(int id, unsigned char *buf)
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
	case ENTER_ROOM:
		ProcessEnterRoomPacket(id, buf);
		break;
	default:
		break;
	}
}

// ä�� �̵� ��Ŷ ó��
void User::ProcessChangeChannelPacket(int id, unsigned char *buf)
{
	// ChattingServer Ŭ������ ������� mClients�� �����;� �Ѵ�.
	// �ذ�� : ChattingServer Ŭ������ �̱������� �����.
	int _currentChannelIndex = GetChannelIndex();
	User *userInfo = ChattingServer::GetInstance()->GetUserInfo();

	Change_Channel *packet = reinterpret_cast<Change_Channel*>(buf);
	Singletone::GetInstance()->channel[_currentChannelIndex].DeleteUserIndex(userInfo);
	Singletone::GetInstance()->channel[packet->channelIndex].AddUserIndex(userInfo);


	std::cout << "[" << id << "] ������ " << "[" << packet->channelIndex
		<< "] �� ä�η� �̵��߽��ϴ�." << std::endl;
}

// �� ���� ��Ŷ ó��
void User::ProcessCreateRoomPacket(int id, unsigned char *buf)
{
	User *userInfo = ChattingServer::GetInstance()->GetUserInfo();
	Create_Room *packet = reinterpret_cast<Create_Room*>(buf);
	
	// ���ο� �� ���� (����-> ������ �濡 ������ ���?)
	// ��� �̱��� ������ �����ϴ� ���� ���� �ȵ��. -> ���?

	// ���� �ϳ��� ���� ���� �ٷ� �����Ѵ�.
	if (Singletone::GetInstance()->roomList.size() == 0) {
		Singletone::GetInstance()->roomList.emplace_back(packet->roomIndex);

		SendNotifyExistRoomPacket(packet->roomIndex, false);

		Room *newRoom = new Room(packet->roomIndex, userInfo->channel_index);	// ���ο� �� ����
		Singletone::GetInstance()->roomList.emplace_back(packet->roomIndex);
		//Channel::GetInstance()->AddNewRoom(packet->roomIndex, newRoom);			
		Singletone::GetInstance()->channel[userInfo->channel_index].AddNewRoom(packet->roomIndex, newRoom); // ä�ο� �� �߰�

		std::cout << "[" << id << "] ������ " << "[" << packet->roomIndex
			<< "] �� ���� �����Ͽ����ϴ�." << std::endl;
	}

	// ���� �ϳ� �̻��� ��
	else {		
		// �̹� �����ϴ� ������ üũ�Ѵ�. �̹� �����ϸ� �� ������ ���� �ʴ´�.
		for (auto iter = Singletone::GetInstance()->roomList.begin(); iter != Singletone::GetInstance()->roomList.end(); ++iter)
		{
			if ((*iter) == packet->roomIndex)
			{
				std::cout << packet->roomIndex << "�� ���� �̹� �ֽ��ϴ�." << std::endl;

				SendNotifyExistRoomPacket(packet->roomIndex, true);
				return;
			}
		}

		// �����ϴ� ���� ���� ��
		Room *newRoom = new Room(packet->roomIndex, userInfo->channel_index);	// ���ο� �� ����
		Singletone::GetInstance()->roomList.emplace_back(packet->roomIndex);
		//Channel::GetInstance()->AddNewRoom(packet->roomIndex, newRoom);			
		Singletone::GetInstance()->channel[userInfo->channel_index].AddNewRoom(packet->roomIndex, newRoom); // ä�ο� �� �߰�

		SendNotifyExistRoomPacket(packet->roomIndex, false);

		std::cout << "[" << id << "] ������ " << "[" << packet->roomIndex
			<< "] �� ���� �����Ͽ����ϴ�." << std::endl;
	}	

}

// ä�ù� ���� ��Ŷ ó�� -> �����Ǿ� �ִ� ���� ����Ʈ�� �����ش�.
void User::ProcessEnterRoomPacket(int id, unsigned char * buf)
{
	SendRoomListPacket();
	Enter_Room *enter_packet = reinterpret_cast<Enter_Room*>(buf);

	
}

// ä�� �޽��� ��Ŷ ó��
void User::ProcessRoomChattingPacket(int id, unsigned char *buf)
{
	Room_Chatting *room_packet = reinterpret_cast<Room_Chatting*>(buf);

	//for(auto user : )
}

