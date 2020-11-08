#pragma once
#include "Acceptor.h"
#include "Socket.h"
/*
	�츮 ���������� �� Ŭ������ accept, receivce, send �� �Ѵ�.
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

