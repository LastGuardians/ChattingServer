#include "stdafx.h"



Room::Room()
{
	memcpy(channelArr, Singletone::GetInstance()->channel, sizeof(Singletone::GetInstance()->channel));

}


Room::~Room()
{
}
