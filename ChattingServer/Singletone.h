#pragma once

class Singletone
{
public:
	Singletone();
	virtual ~Singletone();

	DECLARE_SINGLETON(Singletone);

public:
	Channel *channel =	new Channel[5];
	std::vector<int>	roomList;				// ������ �� ����Ʈ

	//Room *gRoom = new Room[10];
	//std::vector<Room*> gRoom;
	//ChattingServer *serverManage = new ChattingServer();
};

