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

int Room::createRoom(User* userInfo)
{
	userInfo->SetRoomIndex(roomIndex);

	// �濡 ���� ���� 
	//userList.push_back(&userInfo);

	return 0;
}

void Room::SetChannelIndex(int id)
{
	//tutorial::Person* person;
	//tutorial::Person::PhoneNumber* phone_number = person->add_phone();
	//phone_number->set_number("010-111-1111");

	//person->name = "song";
	
}

// �濡 ������ ���� ó��
void Room::SetUserInfo(User* user)
{
	userList.emplace_back(user);
}