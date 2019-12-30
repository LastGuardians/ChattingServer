#include "stdafx.h"

ClientManager::ClientManager()
{
	_pidclients.clear();
}


ClientManager::~ClientManager()
{
}

bool ClientManager::AppendClient(int64_t userpid, std::shared_ptr<ClientInfo> userInfo)
{
	_clientlock.lock();

	auto find = _pidclients.find(userpid);
	if (find != _pidclients.end())
	{
		std::cout << "userpid(%d) client is already exist." << userpid << std::endl;
		return false;
	}

	auto ret = _pidclients.insert(std::make_pair(userInfo->GetUserPid(), userInfo));
	
	return ret.second;
}

bool ClientManager::RemoveClient(int64_t userpid)
{
	_clientlock.lock();

	auto find = _pidclients.find(userpid);
	if (find == _pidclients.end())
	{
		std::cout << "userpid(%d) client is already erase." << userpid << std::endl;
		return false;
	}

	_pidclients.erase(userpid);

	return true;
}

void ClientManager::allClientsLogout()
{
	_clientlock.lock();

	for (auto& client : _pidclients)
	{
		//CNetwork::GetInstance()->Close(client.second->);
	}
}
