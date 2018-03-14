#include "stdafx.h"

IMPLEMENT_SINGLETON(Singletone);

Singletone::Singletone()
{
	roomList.reserve(50);
}

 
Singletone::~Singletone()
{
}
