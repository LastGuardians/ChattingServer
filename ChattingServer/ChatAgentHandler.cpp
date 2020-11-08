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

	// 방이 하나도 없을 때는 바로 생성한다.
	// TODO : channelManager로 수정하기
	//if (true == _channel[userInfo->GetChannelIndex()]->GetRoomIsEmpty())
	//{
	//	exist_room.set_exist(false);
	//	SendPacketAssemble(id, exist_room.type(), exist_room);

	//	_channel[userInfo->GetChannelIndex()]->AddNewRoom(message.roomindex(), userInfo->GetChannelIndex());  // 채널에 방 추가
	//	_channel[userInfo->GetChannelIndex()]->AddUserToRoom(message.roomindex(), userInfo);	// 방에 유저 추가
	//	userInfo->SetRoomIndex(message.roomindex());

	//	std::cout << "[" << id << "] 유저가 " << "[" << message.roomindex()
	//		<< "] 번 방을 생성하였습니다." << std::endl;
	//}

	//// 방이 하나 이상일 때
	//else
	//{
	//	// 이미 존재하는 방인지 체크한다. 이미 존재하면 방 생성을 하지 않는다.
	//	if (true == _channel[userInfo->GetChannelIndex()]->GetRoomIsExist(message.roomindex()))
	//	{
	//		std::cout << message.roomindex() << "번 방이 이미 있습니다." << std::endl;

	//		exist_room.set_exist(true);
	//		SendPacketAssemble(id, exist_room.type(), exist_room);
	//		return;
	//	}

	//	// 존재하는 방이 없을 때
	//	_channel[userInfo->GetChannelIndex()]->AddNewRoom(message.roomindex(), userInfo->GetChannelIndex()); // 채널에 방 추가
	//	_channel[userInfo->GetChannelIndex()]->AddUserToRoom(message.roomindex(), userInfo);	// 방에 유저 추가
	//	userInfo->SetRoomIndex(message.roomindex());

	//	exist_room.set_exist(false);
	//	SendPacketAssemble(id, exist_room.type(), exist_room);

	//	std::cout << "[" << id << "] 유저가 " << "[" << message.roomindex()
	//		<< "] 번 방을 생성하였습니다." << std::endl;
	//}
}
