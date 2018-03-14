#pragma once

#define MSG_SIZE		1024
#define MAX_ROOM_SIZE	50

enum PacketType {
	ENTER_CHANNEL		= 1,
	LEAVE_CHANNEL		= 2,
	CREATE_ROOM			= 3,
	CHANGE_CHANNEL		= 4,
	NOTIFY_ENTER_ROOM	= 5,
	NOTIFY_LEAVE_ROOM	= 6,
	NOTIFY_EXIST_ROOM	= 7,
	ROOM_CHATTING		= 8,
	ENTER_ROOM			= 9,
	ROOM_LIST			= 10,
	//EXIT_SERVER			= 10
};

#pragma pack(push, 1)

struct Enter_Channel {
	BYTE			type = { ENTER_CHANNEL };
	BYTE			size;
	unsigned int	id;
	unsigned int	channelIndex; // (0~4 »çÀÌ°ª)
};

struct Leave_Channel {
	BYTE			type = { LEAVE_CHANNEL };
	BYTE			size;
	unsigned int	id;
	unsigned int	channelIndex;
};

struct Change_Channel {
	BYTE			type = { CHANGE_CHANNEL };
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

struct Room_Chatting {
	BYTE			type = { ROOM_CHATTING };
	BYTE			size;
	unsigned int	id;
	unsigned int	roomIndex;
	char			message[MSG_SIZE];
};

struct Enter_Room {
	BYTE			type = { ENTER_ROOM };
	BYTE			size;
	unsigned int	id;
	unsigned int	roomIndex;
};

struct Notify_Exist_Room {
	BYTE			type = { NOTIFY_EXIST_ROOM };
	BYTE			size;
	unsigned int	id;
	unsigned int	roomIndex;
	bool			exist;
};

struct Room_List {
	BYTE			type = { ROOM_LIST };
	BYTE			size;
	unsigned int	id;
	int				roomList[MAX_ROOM_SIZE];
	int				roomCnt;
};


#pragma pack(pop)