#include "stdafx.h"

Channel::Channel() : _maxRoom{ 5 }, _maxUser{ 10 }
{
}


Channel::Channel(int num)
{
	_channelNo = num;
}

Channel::~Channel()
{
}

bool Channel::GetRoomIsEmpty()
{
	if (_roomList.empty())
		return true;
	return false;
}

bool Channel::GetRoomIsExist(int roomIndex)
{
	for (auto iter = _roomList.begin(); iter != _roomList.end(); ++iter)
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
	_userList.insert(_userList.end(), user);
}

// 유저 삭제
void Channel::DeleteUserToChannel(User* user)
{
	for (auto iter = _userList.begin(); iter != _userList.end(); ++iter)
	{
		if ((*iter)->GetUserId() == user->GetUserId())
		{
			iter = _userList.erase(iter);
			return;
		}
	}
}

bool Channel::AddNewRoom(int roomIndex, int channelIndex)
{	
	for (auto iter = _roomList.begin(); iter != _roomList.end(); ++iter)
	{
		if (iter->first == roomIndex)
		{
			std::cout << roomIndex << "번 방이 이미 존재합니다." << std::endl;
			return false;
		}
	}
	Room *newRoom = new Room(roomIndex, channelIndex);
	_roomList[roomIndex] = *newRoom;
	return true;
}

void Channel::AddUserToRoom(int roomIndex, User * user)
{
	_roomList[roomIndex].AddUserInfo(user);
}

void Channel::DeleteUserToRoom(int roomIndex, User * user)
{
	_roomList[roomIndex].DeleteUserInfo(user);
}