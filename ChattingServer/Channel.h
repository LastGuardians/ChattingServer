#pragma once
#include <list>
#include "Room.h"

// 채널 클래스
class Channel
{
public:
	Channel();
	Channel(int num);
	virtual ~Channel();

	DECLARE_SINGLETON(Channel);

private:
	int								maxRoom;
	int								maxUser;
	int								channelNo;
	std::list<User*>				userList;		// 채널 유저 리스트
	std::unordered_map<int, Room>   roomList;		// 방 생성되면 여기에 추가

public:
	inline int		GetMaxRoom()	{ return maxRoom; }
	inline int		GetChannelNo()	{ return channelNo; }
	
	void			AddUserIndex(User* user);
	void			DeleteUserIndex(User* user);
	void			AddNewRoom(int roomIndex, Room* room);
	//Room*			GetChannelRoomInfo() { return roomList; }

	void			NotifyCreateRoom();			// 채팅룸 생성 알림
	void			NotifyEnterChannel();		// 채널 입장 알림
	bool			NotifyLeaveChannel();		// 채널 퇴장 알림

};

