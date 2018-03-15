#pragma once



// 통신 클래스
class ChattingServer
{
public:
	ChattingServer();
	virtual ~ChattingServer();

	DECLARE_SINGLETON(ChattingServer);

private:
	User			mClients[MAX_USER];
	int				clientId = { -1 };
	HANDLE			m_hiocp = { 0 };
	bool			m_b_server_shut_down = { false };


public:	

	void			err_display(char *msg, int err_no);
	
	void			SetUserInfo(User* user);
	inline int		GetClientID() { return clientId; }
	User*			GetUserInfo(int clientId);

	void		InitServer();
	void		ReleaseServer();

	void		AcceptThread();
	void		WorkerThread();
};

