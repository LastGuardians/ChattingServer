#pragma once

#define MSG_SIZE 1024

enum PacketType {
	ENTER_CHANNEL		= 1,
	LEAVE_CHANNEL		= 2,
	CREATE_ROOM			= 3,
	CHANGE_CHANNEL		= 4,
	NOTIFY_ENTER_ROOM	= 5,
	NOTIFY_LEAVE_ROOM	= 6,
	NOTIFY_EXIST_ROOM	= 7,
	ROOM_CHATTING		= 8,
	//EXIT_SERVER			= 10
};

#pragma pack(push, 1)

struct Enter_Channel {
	BYTE			type = { ENTER_CHANNEL };
	BYTE			size;
	unsigned int	id;
	unsigned int	channelIndex; // (0~4 ���̰�)
};

struct Leave_Channel {
	BYTE			type = { LEAVE_CHANNEL };
	BYTE			size;
	unsigned int	id;
	unsigned int	channelIndex;
};

struct Create_Room {
	BYTE			type = { CREATE_ROOM };
	BYTE			size;
	unsigned int	id;
	unsigned int	roomIndex;
};

struct Change_Channel {
	BYTE			type = { CHANGE_CHANNEL };
	BYTE			size;
	unsigned int	id;
	unsigned int	channelIndex;
};

struct Notify_Exist_Room {
	BYTE			type = { NOTIFY_EXIST_ROOM };
	BYTE			size;
	unsigned int	id;
	unsigned int	roomIndex;
	bool			exist;
};

struct Room_Chatting {
	BYTE			type = { ROOM_CHATTING };
	BYTE			size;
	unsigned int	id;
	unsigned int	roomIndex;
	char			message[MSG_SIZE];
};

#pragma pack(pop)