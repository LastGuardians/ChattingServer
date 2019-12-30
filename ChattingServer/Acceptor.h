#pragma once

class Socket;

class Acceptor
{
public:
	Acceptor();
	virtual ~Acceptor();

	bool Start();
	bool Stop();

private:
	std::shared_ptr<Socket> _socket;
};

