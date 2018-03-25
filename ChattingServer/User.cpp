#include "stdafx.h"

User::User() : userSocket{0}, userId{-1}
{
	
}

User::User(SOCKET s, bool connect, int id)
{
	userSocket = s;
	userId = id;
	userConnected = connect;
}


User::~User()
{

}

void User::SetChannelIndex(int id)
{
	channel_index = id;
}

void User::SetRoomIndex(int room)
{
	room_index = room;
}
