#include "stdafx.h"

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

bool Channel::GetRoomIsEmpty()
{
	if (roomList.empty())
		return true;
	return false;
}

bool Channel::GetRoomIsExist(int roomIndex)
{
	for (auto iter = roomList.begin(); iter != roomList.end(); ++iter)
	{
		if (iter->first == roomIndex)		// 해당 방이 존재함.
		{
			return true;
		}
	}
	return false;
}

// 채널에 유저 삽입
void Channel::AddUserToChannel(User* user)
{
	userList.insert(userList.end(), user);
}

// 유저 삭제
void Channel::DeleteUserToChannel(User* user)
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
	for (auto iter = roomList.begin(); iter != roomList.end(); ++iter)
	{
		if (iter->first == roomIndex)
		{
			std::cout << roomIndex << "번 방이 이미 존재합니다." << std::endl;
			return;
		}
	}
	roomList[roomIndex] = *room;
}

void Channel::AddUserToRoom(int roomIndex, User * user)
{
	roomList[roomIndex].AddUserInfo(user);
}

void Channel::DeleteUserToRoom(int roomIndex, User * user)
{
	roomList[roomIndex].DeleteUserInfo(user);
}