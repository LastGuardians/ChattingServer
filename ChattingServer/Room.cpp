#include "stdafx.h"



Room::Room()
{
}

Room::Room(int room, int channeI) : _roomIndex{ room }, _channelIndex{ channeI }
{

}


Room::~Room()
{
}

// 방에 유저 추가
void Room::AddUserInfo(User* user)
{
	_userList.push_back(user);
}

// 방에서 유저 삭제
void Room::DeleteUserInfo(User * user)
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
