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
	int								_maxRoom;
	int								_maxUser;
	int								_channelNo;
	std::list<User*>				_userList;		// ä�� ���� ����Ʈ
	std::unordered_map<int, Room>   _roomList;		// �� �����Ǹ� ���⿡ �߰�
	
public:
	bool			GetRoomIsEmpty();						// ������ ���� �ϳ��� ���� �� Ȯ��
	bool			GetRoomIsExist(int roomIndex);			// �ش� ���� �����ϴ� �� Ȯ��
	
	void			AddUserToChannel(User* user);					// ä�ο� ���� �߰�
	void			DeleteUserToChannel(User* user);				// ä�ο��� ���� ����
	bool			AddNewRoom(int roomIndex, int channelIndex);	// ���ο� �� �߰�
	void			AddUserToRoom(int roomIndex, User* user);		// �濡 ���� �߰�
	void			DeleteUserToRoom(int roomIndex, User* user);	// �濡�� ���� ����
	
};

