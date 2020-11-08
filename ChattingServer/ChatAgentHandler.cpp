#include "ChatAgentHandler.h"
#include "stdafx.h"

ChatAgentHandler::ChatAgentHandler()
{
}

ChatAgentHandler::~ChatAgentHandler()
{
}

void ChatAgentHandler::Handle_create_room_req(int userpid, const Protocols::Create_Room & message) const
{
	const auto& userInfo = ClientManager::GetInstance()->GetClientByPid(userpid);
	if (nullptr == userInfo)
	{
		std::cout << "userInfo is null! user_pid(%d)" << userpid << std::endl;
		return;
	}

	Protocols::Notify_Exist_Room exist_room;
	exist_room.set_id(userpid);
	exist_room.set_roomindex(message.roomindex());
	exist_room.set_type(Protocols::NOTIFY_EXIST_ROOM);

	// ���� �ϳ��� ���� ���� �ٷ� �����Ѵ�.
	// TODO : channelManager�� �����ϱ�
	//if (true == _channel[userInfo->GetChannelIndex()]->GetRoomIsEmpty())
	//{
	//	exist_room.set_exist(false);
	//	SendPacketAssemble(id, exist_room.type(), exist_room);

	//	_channel[userInfo->GetChannelIndex()]->AddNewRoom(message.roomindex(), userInfo->GetChannelIndex());  // ä�ο� �� �߰�
	//	_channel[userInfo->GetChannelIndex()]->AddUserToRoom(message.roomindex(), userInfo);	// �濡 ���� �߰�
	//	userInfo->SetRoomIndex(message.roomindex());

	//	std::cout << "[" << id << "] ������ " << "[" << message.roomindex()
	//		<< "] �� ���� �����Ͽ����ϴ�." << std::endl;
	//}

	//// ���� �ϳ� �̻��� ��
	//else
	//{
	//	// �̹� �����ϴ� ������ üũ�Ѵ�. �̹� �����ϸ� �� ������ ���� �ʴ´�.
	//	if (true == _channel[userInfo->GetChannelIndex()]->GetRoomIsExist(message.roomindex()))
	//	{
	//		std::cout << message.roomindex() << "�� ���� �̹� �ֽ��ϴ�." << std::endl;

	//		exist_room.set_exist(true);
	//		SendPacketAssemble(id, exist_room.type(), exist_room);
	//		return;
	//	}

	//	// �����ϴ� ���� ���� ��
	//	_channel[userInfo->GetChannelIndex()]->AddNewRoom(message.roomindex(), userInfo->GetChannelIndex()); // ä�ο� �� �߰�
	//	_channel[userInfo->GetChannelIndex()]->AddUserToRoom(message.roomindex(), userInfo);	// �濡 ���� �߰�
	//	userInfo->SetRoomIndex(message.roomindex());

	//	exist_room.set_exist(false);
	//	SendPacketAssemble(id, exist_room.type(), exist_room);

	//	std::cout << "[" << id << "] ������ " << "[" << message.roomindex()
	//		<< "] �� ���� �����Ͽ����ϴ�." << std::endl;
	//}
}
