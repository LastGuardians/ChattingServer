#pragma once
#include <list>
#include "Room.h"

// ä�� Ŭ����
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
	std::list<User*>				userList;		// ä�� ���� ����Ʈ
	std::unordered_map<int, Room>   roomList;		// �� �����Ǹ� ���⿡ �߰�

public:
	inline int		GetMaxRoom()	{ return maxRoom; }
	inline int		GetChannelNo()	{ return channelNo; }
	
	void			AddUserIndex(User* user);
	void			DeleteUserIndex(User* user);
	void			AddNewRoom(int roomIndex, Room* room);
	//Room*			GetChannelRoomInfo() { return roomList; }

	void			NotifyCreateRoom();			// ä�÷� ���� �˸�
	void			NotifyEnterChannel();		// ä�� ���� �˸�
	bool			NotifyLeaveChannel();		// ä�� ���� �˸�

};

