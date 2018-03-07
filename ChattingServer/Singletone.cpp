#include "stdafx.h"

IMPLEMENT_SINGLETON(Singletone);

Singletone::Singletone()
{
	roomList.reserve(30);
}


Singletone::~Singletone()
{
}
