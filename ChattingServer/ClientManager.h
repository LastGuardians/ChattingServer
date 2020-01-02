/*
	author : songykim
	connection을 맺은 모든 ClientInfo 메모리를 관리하는 매니저 class
	ClientInfo 객체를 Get 하는게 주목적이다. - lock 필수
*/

#pragma once
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <memory>

#include "ClientInfo.h"

class ClientManager
{
public:
	ClientManager();
	virtual ~ClientManager();

	bool AppendClient(int64_t userpid, std::shared_ptr<ClientInfo> userInfo);
	bool RemoveClient(int64_t userpid);

	void allClientsLogout();

private:

	std::shared_mutex _clientlock;	
	std::unordered_map<int64_t, std::shared_ptr<ClientInfo>> _pidclients;
};

