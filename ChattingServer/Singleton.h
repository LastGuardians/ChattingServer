#pragma once

class Singleton
{
public:
	Singleton();
	virtual ~Singleton();

	DECLARE_SINGLETON(Singleton);

public:
	Channel *channel =	new Channel[10];	// 채널을 정적배열로 잡아놓은게 맘에 안든다.(channelManager에서 addChannel(channel 컨테이너에 insert)하는 방식 고려)
	
};

