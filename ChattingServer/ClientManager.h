/*
	author : songykim
	connection�� ���� ��� ClientInfo �޸𸮸� �����ϴ� �Ŵ��� class
	ClientInfo ��ü�� Get �ϴ°� �ָ����̴�. - lock �ʼ�
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

	DECLARE_SINGLETON(ClientManager);

	bool AppendClient(int64_t userpid, std::shared_ptr<ClientInfo> userInfo);
	bool RemoveClient(int64_t userpid);
	std::shared_ptr<ClientInfo> GetClientByPid(int64_t userPid);

	void allClientsLogout();

private:

	std::shared_mutex _clientlock;	
	std::unordered_map<int64_t, std::shared_ptr<ClientInfo>> _pidclients;
};

