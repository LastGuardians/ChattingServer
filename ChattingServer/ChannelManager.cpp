#include "stdafx.h"

//IMPLEMENT_SINGLETON(ChannelManager);

ChannelManager::ChannelManager()
{
	//roomList.reserve(50);
	_channel.reserve(50);

}

 
ChannelManager::~ChannelManager()
{
}

bool ChannelManager::addChannel(Channel* channel)
{
	// �ش� ä���� ������ 
	if (nullptr == getChannel(channel->GetChannelNo()))
	{
		_channel.emplace_back(channel);
		return true;
	}
	
	return false;
}

Channel* ChannelManager::getChannel(int channelNo)
{
	//_channel = { 0 };

	// �ش��ϴ� ä�� ��ü ����
	if (_channel.size() == 0)
		return nullptr;
	
	for (auto channel : _channel)
	{
		if (channelNo == channel->GetChannelNo())
		{
			return channel;
		}
	}

	return nullptr;
}
