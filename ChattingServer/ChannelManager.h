#pragma once

class ChannelManager
{

public:
	ChannelManager();
	virtual ~ChannelManager();

	//DECLARE_SINGLETON(ChannelManager);

private:
	std::vector<Channel*> _channel;
	//std::vector<int>	roomList;				// ������ �� ����Ʈ
		
public:
	bool		addChannel(Channel* channel);
	Channel*	getChannel(int channelNo);
};

