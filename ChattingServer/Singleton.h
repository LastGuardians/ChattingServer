#pragma once

class Singleton
{
public:
	Singleton();
	virtual ~Singleton();

	DECLARE_SINGLETON(Singleton);

public:
	Channel *channel =	new Channel[5];
	//std::vector<int>	roomList;				// ������ �� ����Ʈ
};

