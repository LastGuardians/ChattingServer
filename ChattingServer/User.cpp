#include "stdafx.h"

User::User() : _userSocket{0}, _userId{-1}
{
	
}

User::User(SOCKET s, bool connect, int id)
{
	_userSocket = s;
	_userId = id;
}


User::~User()
{

}

void User::SetChannelIndex(int id)
{
	_channel_index = id;
}

void User::SetRoomIndex(int room)
{
	_room_index = room;
}
