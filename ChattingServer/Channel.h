#pragma once
#include <list>
#include "Room.h"

// 채널 클래스
class Channel
{
	friend class ChannelManager;

public:
	Channel();
	Channel(int channelId);
	virtual ~Channel();	

private:
	ChannelManager*					_channelManager;		// 채널 관리해주는 객체

	int								maxRoom;
	int								maxUser;
	int								channelNo;
	std::unordered_map<int, User*>	userList;		// 채널 유저 리스트 -> 없어도 되지 않을까?
	std::unordered_map<int, Room*>  roomList;		// 방 생성되면 여기에 추가 (key : 방 인덱스 / value : 방 객체)

public:
	
	bool				CreateRoom(int roomIndex, Room * room);

	inline int			GetMaxRoom()	{ return maxRoom; }
	inline int			GetChannelNo()	{ return channelNo; }
	
	void				enterChannel(User* user);
	void				leaveChannel(User* user);

	void				AddNewRoom(int roomIndex, Room* room);
	//Room*				GetChannelRoomInfo() { return roomList; }

	void				NotifyCreateRoom();			// 채팅룸 생성 알림
	void				NotifyEnterChannel();		// 채널 입장 알림
	bool				NotifyLeaveChannel();		// 채널 퇴장 알림


	inline ChannelManager* Channel::channelManager() { return _channelManager; }	// 채널 매니저 객체 리턴
	inline std::unordered_map<int, Room*> GetRoomInfo() { return roomList; }		// 채팅룸 리스트 리턴

};

