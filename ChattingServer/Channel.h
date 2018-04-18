#pragma once
#include <list>
#include "Room.h"

// 채널 클래스 -> 채널 매니저와 비슷한 역할을 한다.(room, channel을 관리)
class Channel
{
public:
	Channel();
	Channel(int num);
	virtual ~Channel();

private:
	int								maxRoom;
	int								maxUser;
	int								channelNo;
	std::list<User*>				userList;		// 채널 유저 리스트
	std::unordered_map<int, Room>   roomList;		// 방 생성되면 여기에 추가(key : 방 번호, value : 방 객체)
	
public:
	inline int		GetMaxRoom()	{ return maxRoom; }
	inline int		GetChannelNo()	{ return channelNo; }
	bool			GetRoomIsEmpty();						// 생성된 방이 하나도 없는 지 확인
	bool			GetRoomIsExist(int roomIndex);			// 해당 방이 존재하는 지 확인
	
	void			AddUserToChannel(User* user);					// 채널에 유저 추가
	void			DeleteUserToChannel(User* user);				// 채널에서 유저 삭제
	void			AddNewRoom(int roomIndex, Room* room);			// 새로운 방 추가
	void			AddUserToRoom(int roomIndex, User* user);		// 방에 유저 추가
	void			DeleteUserToRoom(int roomIndex, User* user);	// 방에서 유저 삭제
	
};

