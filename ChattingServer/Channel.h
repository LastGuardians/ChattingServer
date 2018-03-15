#pragma once
#include <list>
#include "Room.h"

// ä�� Ŭ����
class Channel
{
	friend class ChannelManager;

public:
	Channel();
	Channel(int channelId);
	virtual ~Channel();	

private:
	ChannelManager*					_channelManager;		// ä�� �������ִ� ��ü

	int								maxRoom;
	int								maxUser;
	int								channelNo;
	std::unordered_map<int, User*>	userList;		// ä�� ���� ����Ʈ -> ��� ���� ������?
	std::unordered_map<int, Room*>  roomList;		// �� �����Ǹ� ���⿡ �߰� (key : �� �ε��� / value : �� ��ü)

public:
	
	bool				CreateRoom(int roomIndex, Room * room);

	inline int			GetMaxRoom()	{ return maxRoom; }
	inline int			GetChannelNo()	{ return channelNo; }
	
	void				enterChannel(User* user);
	void				leaveChannel(User* user);

	void				AddNewRoom(int roomIndex, Room* room);
	//Room*				GetChannelRoomInfo() { return roomList; }

	void				NotifyCreateRoom();			// ä�÷� ���� �˸�
	void				NotifyEnterChannel();		// ä�� ���� �˸�
	bool				NotifyLeaveChannel();		// ä�� ���� �˸�


	inline ChannelManager* Channel::channelManager() { return _channelManager; }	// ä�� �Ŵ��� ��ü ����
	inline std::unordered_map<int, Room*> GetRoomInfo() { return roomList; }		// ä�÷� ����Ʈ ����

};

