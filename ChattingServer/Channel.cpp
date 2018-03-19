#include "stdafx.h"

IMPLEMENT_SINGLETON(Channel);

Channel::Channel() : maxRoom{ 5 }, maxUser{ 10 }
{
}


Channel::Channel(int num)
{
	channelNo = num;
}

Channel::~Channel()
{
}

// 채널에 유저 삽입
void Channel::AddUser(User* user)
{
	userList.insert(userList.end(), user);
}

// 유저 삭제
void Channel::DeleteUser(User* user)
{
	for (auto iter = userList.begin(); iter != userList.end(); ++iter)
	{
		auto info = *iter;
		if (info->GetUserId() == user->GetUserId())
			iter = userList.erase(iter);
		return;
	}
}

void Channel::AddNewRoom(int roomIndex, Room * room)
{
	roomList[roomIndex] = *room;
}

void Channel::NotifyCreateRoom()
{

}