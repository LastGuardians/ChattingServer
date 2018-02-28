#pragma once

// 유저 클래스 (소켓)
class User
{
public:
	User();
	virtual ~User();

private:
	SOCKET userSocket;
	int userId;

public:
	void InitUserInfo(SOCKET s, int id);
};

