#pragma once
class Room
{
public:
	Room();
	virtual ~Room();

private:
	Channel channelArr[5];

public:
	void			CreateRoom();		// ä�÷� ����
	void			EnterChannel();		// ä�� ����
	bool			LeaveChannel();		// ä�� ����
};

