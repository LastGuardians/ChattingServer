#pragma once
#include <list>

// ä�� Ŭ����
class Channel
{
public:
	Channel();
	Channel(int num);
	virtual ~Channel();

private:
	int					maxRoom;
	int					maxUser;
	int					channelNo;
	std::list<User*>	userList;		// ���� ����Ʈ

public:
	inline int		GetMaxRoom()	{ return maxRoom; }
	inline int		GetChannelNo()	{ return channelNo; }
	
	void			AddUserIndex(User* user);
	void			DeleteUserIndex(int id);


};

