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

void User::SetRoomIndex(int room)
{
	room_index = room;
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
	/*Room_List list_packet;

	if (ChannelManager::GetInstance()->roomList.size() == 0)
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
	for (auto iter = ChannelManager::GetInstance()->roomList.begin(); iter != ChannelManager::GetInstance()->roomList.end(); ++iter)
	{
		list_packet.roomList[cnt++] = *iter;
	}
	cnt = 0;

	list_packet.id = userId;
	list_packet.type = ROOM_LIST;
	list_packet.roomCnt = ChannelManager::GetInstance()->roomList.size();
	SendPacket(reinterpret_cast<unsigned char*>(&list_packet));
*/
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

// 채널 이동 패킷 처리
void User::ProcessChangeChannelPacket(int id, unsigned char *buf)
{
	// ChattingServer 클래스의 멤버변수 mClients를 가져와야 한다.
	// 해결법 : ChattingServer 클래스를 싱글턴으로 만든다. -> 싱글턴으로 바꾸는게 아닌 ClientMananger 클래스 만들어서 관리하는게 더 낫다.

	User *userInfo = ChattingServer::GetInstance()->GetUserInfo(id);
	int _currentChannelIndex = userInfo->GetChannelIndex();

	Change_Channel *packet = reinterpret_cast<Change_Channel*>(buf);

	//_channelManager->getChannel(_currentChannelIndex)->leaveChannel(userInfo);
	Channel* _curChannel = _channelManager->getChannel(_currentChannelIndex);
	_curChannel->leaveChannel(userInfo);				// 현재 채널에서 유저를 삭제하고

	Channel* _newChannel = _channelManager->getChannel(packet->channelIndex);
	if (nullptr == _newChannel)						// 채널이 없다면 새로 생성한다.
	{
		Channel channel(packet->channelIndex);
		channel.channelManager()->addChannel(&channel);
	}
	_newChannel->enterChannel(userInfo);			// 이동하려는 채널에 유저 삽입

	std::cout << "[" << id << "] 유저가 " << "[" << packet->channelIndex
		<< "] 번 채널로 이동했습니다." << std::endl;
}

// 방 생성 패킷 처리
void User::ProcessCreateRoomPacket(int id, unsigned char *buf)
{
	User *userInfo = ChattingServer::GetInstance()->GetUserInfo(id);
	Create_Room *packet = reinterpret_cast<Create_Room*>(buf);
	
	auto _currChannel = userInfo->GetChannelIndex();

	// 새로운 방 생성 (추후-> 생성된 방에 접근은 어떻게?)
	// 계속 싱글턴 변수에 접근하는 것이 맘에 안든다. -> 상속?
	Room room(packet->roomIndex, userInfo->GetChannelIndex());
	
	Channel* _channel = _channelManager->getChannel(_currChannel);
	if (true == _channel->CreateRoom(packet->roomIndex, &room))
	{
		_roomMananger->createRoom(userInfo);

		std::cout << "[" << id << "] 유저가 " << "[" << packet->roomIndex
			<< "] 번 방을 생성하였습니다." << std::endl;
	}

	std::cout << packet->roomIndex << "번 방이 이미 있습니다." << std::endl;
	
}

// 채팅방 입장 패킷 처리 -> 생성되어 있는 방의 리스트를 보내준다.
void User::ProcessEnterRoomPacket(int id, unsigned char * buf)
{
	SendRoomListPacket();
	Enter_Room *enter_packet = reinterpret_cast<Enter_Room*>(buf);

	
}

// 채팅 메시지 패킷 처리
void User::ProcessRoomChattingPacket(int id, unsigned char *buf)
{
	Room_Chatting *room_packet = reinterpret_cast<Room_Chatting*>(buf);

	//for(auto user : )
}

