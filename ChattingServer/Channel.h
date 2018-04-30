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

private:
	int								_maxRoom;
	int								_maxUser;
	int								_channelNo;
	std::list<User*>				_userList;		// 채널 유저 리스트
	std::unordered_map<int, Room>   _roomList;		// 방 생성되면 여기에 추가
	
public:
	bool			GetRoomIsEmpty();						// 생성된 방이 하나도 없는 지 확인
	bool			GetRoomIsExist(int roomIndex);			// 해당 방이 존재하는 지 확인
	
	void			AddUserToChannel(User* user);					// 채널에 유저 추가
	void			DeleteUserToChannel(User* user);				// 채널에서 유저 삭제
	bool			AddNewRoom(int roomIndex, int channelIndex);	// 새로운 방 추가
	void			AddUserToRoom(int roomIndex, User* user);		// 방에 유저 추가
	void			DeleteUserToRoom(int roomIndex, User* user);	// 방에서 유저 삭제
	
};

