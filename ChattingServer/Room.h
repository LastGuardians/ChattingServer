#pragma once
class Room
{
public:
	Room();
	Room(int roomIndex, int channelIndex);
	virtual ~Room();

private:
	int							_roomIndex;				// 방 인덱스
	int							_channelIndex;
	std::list<User*>			_userList;				// 방 유저 리스트
	
	
public:
	inline int GetRoomIndex() { return _roomIndex; }
		
	void AddUserInfo(User* user);
	void DeleteUserInfo(User* user);
};

