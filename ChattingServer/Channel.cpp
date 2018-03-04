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

// ä�ο� ���� ����
void Channel::AddUserIndex(User* user)
{
	userList.insert(userList.end(), user);
}

// ���� ����
void Channel::DeleteUserIndex(int id)
{
	for (auto i = userList.begin(); i != userList.end(); ++i)
	{
		if((*i)->GetUserId() == id)
			userList.erase(i);
	}
}