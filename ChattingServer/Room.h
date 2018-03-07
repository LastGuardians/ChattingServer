#pragma once
class Room
{
public:
	Room();
	virtual ~Room();

private:
	Channel channelArr[5];

public:
	void			CreateRoom();		// 채팅룸 생성
	void			EnterChannel();		// 채널 입장
	bool			LeaveChannel();		// 채널 퇴장
};

