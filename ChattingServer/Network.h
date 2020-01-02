#pragma once
#include "Acceptor.h"
#include "Socket.h"

class CNetwork
{
	friend class Socket;

public:
	CNetwork();
	virtual ~CNetwork();

	DECLARE_SINGLETON(CNetwork);

	bool Start();
	void Stop();	

	bool Send(Socket* handle, google::protobuf::Message * msg);
	bool SendBroadcast(google::protobuf::Message * msg);
	bool Close(Socket* handle);

private:

	Acceptor  _acceptor;
};

