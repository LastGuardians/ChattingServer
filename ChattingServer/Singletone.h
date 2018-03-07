#pragma once

class Singletone
{
public:
	Singletone();
	virtual ~Singletone();

	DECLARE_SINGLETON(Singletone);

public:
	Channel *channel = new Channel[5];
	std::vector<Room*> roomList;
};

