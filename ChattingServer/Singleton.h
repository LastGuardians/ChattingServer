#pragma once

class Singleton
{
public:
	Singleton();
	virtual ~Singleton();

	DECLARE_SINGLETON(Singleton);

public:
	Channel *channel =	new Channel[10];	// ä���� �����迭�� ��Ƴ����� ���� �ȵ��.(channelManager���� addChannel(channel �����̳ʿ� insert)�ϴ� ��� ���)
	
};

