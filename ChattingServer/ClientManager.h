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

