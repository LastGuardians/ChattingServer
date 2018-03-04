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

// 채널에 유저 삽입
void Channel::AddUserIndex(User* user)
{
	userList.insert(userList.end(), user);
}

// 유저 삭제
void Channel::DeleteUserIndex(int id)
{
	for (auto i = userList.begin(); i != userList.end(); ++i)
	{
		if((*i)->GetUserId() == id)
			userList.erase(i);
	}
}