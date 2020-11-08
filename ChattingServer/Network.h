#pragma once
#include "Acceptor.h"
#include "Socket.h"
/*
	우리 서버에서는 이 클래스가 accept, receivce, send 다 한다.
*/
class CNetwork
{
	friend class CSocket;

public:
	CNetwork();
	virtual ~CNetwork();

	DECLARE_SINGLETON(CNetwork);

	bool Start(CSocket* handle);
	void Stop();	

	bool Send(CSocket* handle, google::protobuf::Message * msg);
	bool SendBroadcast(google::protobuf::Message * msg);
	bool Close(SOCKET* handle);

private:
	bool GetLocalHostAddr(SOCKADDR_IN& OUT addr);

};

