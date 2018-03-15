#pragma once
class Room
{
public:
	Room();
	Room(int roomIndex, int channelIndex);
	virtual ~Room();

private:
	int							roomIndex;				// 방 인덱스
	int							channelIndex;
	std::list<User*>			userList;				// 방 유저 리스트
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

