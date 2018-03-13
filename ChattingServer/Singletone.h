#pragma once

class Singletone
{
public:
	Singletone();
	virtual ~Singletone();

	DECLARE_SINGLETON(Singletone);

public:
	Channel *channel =	new Channel[5];
	std::vector<int>	roomList;				// 积己等 规 府胶飘

	//Room *gRoom = new Room[10];
	//std::vector<Room*> gRoom;
	//ChattingServer *serverManage = new ChattingServer();
};

