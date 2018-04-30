#pragma once
class Room
{
public:
	Room();
	Room(int roomIndex, int channelIndex);
	virtual ~Room();

private:
	int							_roomIndex;				// �� �ε���
	int							_channelIndex;
	std::list<User*>			_userList;				// �� ���� ����Ʈ
	
	
public:
	inline int GetRoomIndex() { return _roomIndex; }
		
	void AddUserInfo(User* user);
	void DeleteUserInfo(User* user);
};

