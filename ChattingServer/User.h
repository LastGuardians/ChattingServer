#pragma once

// ���� Ŭ���� (����)
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

