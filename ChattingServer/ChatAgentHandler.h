#pragma once
#include "Protocols.pb.h"

class ChatAgentHandler
{
public:
	ChatAgentHandler();
	virtual ~ChatAgentHandler();
	
	void		Handle_create_room_req(int id, const Protocols::Create_Room& message) const;
	void		Handle_change_channel_req(int id, const Protocols::Change_Channel& message) const;
	void		Handle_room_chatting_req(int id, const Protocols::Room_Chatting& message) const;
	void		Handle_enter_room_req(int id, const Protocols::Enter_Room& message) const;
	void		Handle_channel_chatting_req(int id, const Protocols::Channel_Chatting& message) const;
	void		Handle_leave_room_req(int id, const Protocols::Leave_Room& message) const;
	void		Handle_room_user_list_req(int id, const Protocols::Room_List& message) const;
};

