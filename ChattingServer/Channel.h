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

private:
	int								maxRoom;
	int								maxUser;
	int								channelNo;
	std::list<User*>				userList;		// ä�� ���� ����Ʈ
	std::unordered_map<int, Room>   roomList;		// �� �����Ǹ� ���⿡ �߰�
	
public:
	inline int		GetMaxRoom()	{ return maxRoom; }
	inline int		GetChannelNo()	{ return channelNo; }
	bool			GetRoomIsEmpty();						// ������ ���� �ϳ��� ���� �� Ȯ��
	bool			GetRoomIsExist(int roomIndex);			// �ش� ���� �����ϴ� �� Ȯ��
	
	void			AddUserToChannel(User* user);					// ä�ο� ���� �߰�
	void			DeleteUserToChannel(User* user);				// ä�ο��� ���� ����
	void			AddNewRoom(int roomIndex, Room* room);			// ���ο� �� �߰�
	void			AddUserToRoom(int roomIndex, User* user);		// �濡 ���� �߰�
	void			DeleteUserToRoom(int roomIndex, User* user);	// �濡�� ���� ����

	//Room*			GetChannelRoomInfo() { return roomList; }

};

