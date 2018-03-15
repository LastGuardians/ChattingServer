#pragma once
class Room
{
public:
	Room();
	Room(int roomIndex, int channelIndex);
	virtual ~Room();

private:
	int							roomIndex;				// �� �ε���
	int							channelIndex;
	std::list<User*>			userList;				// �� ���� ����Ʈ
	//Channel*					_channel;
	
	
public:
	inline int GetRoomIndex() { return roomIndex; }
	inline std::list<User*>& GetUserList() { return userList; }

	int		createRoom(User* userInfo);
	int		enterRoom(User* userInfo);
	int		leaveRoom(User* userInfo);


	void	SetChannelIndex(int id);
	void	SetUserInfo(User* user);
	void	SetRoomInfo(Room* room);

	
};

