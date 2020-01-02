#include "stdafx.h"

User::User()
{
	
}

User::User(SOCKET s, __int64 id) :
	_userSocket(s),
	_userId(id)
{
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
