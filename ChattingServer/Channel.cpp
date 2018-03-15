#include "stdafx.h"

//IMPLEMENT_SINGLETON(Channel);

Channel::Channel() : maxRoom{ 5 }, maxUser{ 10 }
{
	_channelManager = new ChannelManager;
}


Channel::Channel(int channelId)
{
	_channelManager = new ChannelManager;
	channelNo = channelId;
}

Channel::~Channel()
{
}

bool Channel::CreateRoom(int roomIndex, Room * room)
{
	for (auto iter = roomList.begin(); iter != roomList.end(); ++iter)
	{
		if ((*iter).first == roomIndex)		// �̹� �ִ� �� �ε������ false
			return false;
	}
	roomList[roomIndex] = room;				// ���� ���̶�� �����Ѵ�.

	return true;
}

void Channel::enterChannel(User * user)
{
	userList[user->GetUserId()] = user;		// ä�� ���� ��ü�� ���� ���� 
	
	user->SetChannelIndex(channelNo);		// �������� ä�� �ε��� ����

}

void Channel::leaveChannel(User * user)
{
	userList.erase(user->GetUserId());		// ä�� ���� ��ü���� ���� ����

	user->SetChannelIndex(NO_CHANNEL);		// ������ ä�� �ε����� �ʱ�ȭ
}

void Channel::AddNewRoom(int roomIndex, Room * room)
{
	//roomList[roomIndex] = *room;
}

void Channel::NotifyCreateRoom()
{

}