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

// �濡 ������ ���� ó��
void Room::SetUserInfo(User* user)
{
	userList.emplace_back(user);
}