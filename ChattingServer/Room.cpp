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

// �濡 ���� �߰�
void Room::AddUserInfo(User* user)
{
	userList.emplace_back(user);
}

// �濡�� ���� ����
void Room::DeleteUserInfo(User * user)
{
	for (auto iter = userList.begin(); iter != userList.end(); ++iter)
	{
		auto info = *iter;
		if (info->GetUserId() == user->GetUserId())
		{
			iter = userList.erase(iter);
			return;
		}			
	}
}
