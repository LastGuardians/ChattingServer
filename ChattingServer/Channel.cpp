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
		if ((*iter).first == roomIndex)		// 이미 있는 방 인덱스라면 false
			return false;
	}
	roomList[roomIndex] = room;				// 없는 방이라면 생성한다.

	return true;
}

void Channel::enterChannel(User * user)
{
	userList[user->GetUserId()] = user;		// 채널 유저 객체에 유저 삽입 
	
	user->SetChannelIndex(channelNo);		// 유저에게 채널 인덱스 셋팅

}

void Channel::leaveChannel(User * user)
{
	userList.erase(user->GetUserId());		// 채널 유저 객체에서 유저 삭제

	user->SetChannelIndex(NO_CHANNEL);		// 유저의 채널 인덱스를 초기화
}

void Channel::AddNewRoom(int roomIndex, Room * room)
{
	//roomList[roomIndex] = *room;
}

void Channel::NotifyCreateRoom()
{

}