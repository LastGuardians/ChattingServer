#pragma once

class CSocket;

class Acceptor
{
public:
	Acceptor();
	virtual ~Acceptor();

	bool Start();
	bool Stop();

private:
	std::shared_ptr<CSocket> _socket;
};

