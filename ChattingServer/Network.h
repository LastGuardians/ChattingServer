#pragma once
#include "Acceptor.h"

class CNetwork
{
public:
	CNetwork();
	virtual ~CNetwork();

	DECLARE_SINGLETON(CNetwork);

	bool Start();
	void Stop();	

	bool Send(SOCKET handle, google::protobuf::Message * msg);
	bool SendBroadcast(google::protobuf::Message * msg);
	bool Close(SOCKET handle);

private:

	Acceptor  _acceptor;
};

