#pragma once

class Singleton
{
public:
	Singleton();
	virtual ~Singleton();

	DECLARE_SINGLETON(Singleton);

public:
	Channel *channel =	new Channel[5];
	std::vector<int>	roomList;				// ������ �� ����Ʈ

	//Room *gRoom = new Room[10];
	//std::vector<Room*> gRoom;
	//ChattingServer *serverManage = new ChattingServer();
};

