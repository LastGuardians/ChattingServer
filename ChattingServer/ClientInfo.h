#pragma once
#include <string>

namespace Lobby
{
	class ClientInfo
	{
	public:
		ClientInfo();
		virtual ~ClientInfo();

		void setUserRoomNo();
		void setUserChannelNo();

	private:
		__int64 _session,
				_userpid;

		int _roomNo,
			_channelNo;

		std::string _userId;
	};
}
