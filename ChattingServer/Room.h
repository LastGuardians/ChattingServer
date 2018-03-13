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
	
	
public:
	inline int GetRoomIndex() { return roomIndex; }

	void SetChannelIndex(int id);
	void SetUserInfo(User* user);
	void SetRoomInfo(Room* room);

};

