#pragma once
#include <list>

// 채널 클래스
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
	std::list<User*>	userList;		// 유저 리스트

public:
	inline int		GetMaxRoom()	{ return maxRoom; }
	inline int		GetChannelNo()	{ return channelNo; }
	
	void			AddUserIndex(User* user);
	void			DeleteUserIndex(int id);


};

