#include "stdafx.h"



Room::Room()
{
}

Room::Room(int room, int channeI) : roomIndex{ room }, channelIndex{ channeI }
{

}


Room::~Room()
{
}

//void Room::SetChannelIndex(int id)
//{
//
//}

// 방에 입장한 유저 처리
void Room::SetUserInfo(User* user)
{
	userList.emplace_back(user);
}