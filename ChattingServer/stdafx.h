#pragma once
#define PROTOBUF_USE_DLLS
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "libprotobufd.lib")

#include <iostream>
#include <thread>
#include <WinSock2.h>
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <thread>
#include <random>
#include <ctime>
#include <unordered_map>
#include <sql.h>

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/text_format.h>
#include "Protocols.pb.h"

#define MAX_USER	50
#define BUFSIZE		1024
#define MSG_SIZE	50

using namespace google;


#define NO_COPY(CLASSNAME)							\
	private:										\
	CLASSNAME(const CLASSNAME&);					\
	CLASSNAME& operator = (const CLASSNAME&);		

#define DECLARE_SINGLETON(CLASSNAME)				\
	NO_COPY(CLASSNAME)								\
	private:										\
	static CLASSNAME*	m_pInstance;				\
	public:											\
	static CLASSNAME*	GetInstance( void );		\
	static void DestroyInstance( void );	

#define IMPLEMENT_SINGLETON(CLASSNAME)				\
	CLASSNAME*	CLASSNAME::m_pInstance = NULL;		\
	CLASSNAME*	CLASSNAME::GetInstance( void )	{	\
		if(NULL == m_pInstance) {					\
			m_pInstance = new CLASSNAME;			\
		}											\
		return m_pInstance;							\
	}												\
	void CLASSNAME::DestroyInstance( void ) {		\
		if(NULL != m_pInstance)	{					\
			delete m_pInstance;						\
			m_pInstance = NULL;						\
		}											\
	}


enum EVENT_TYPE
{
	OV_RECV = 0,
	OV_SEND = 1,
};

#include "protocol.h"
#include "User.h"
#include "ChattingServer.h"
#include "Channel.h"
#include "Room.h"
#include "Singleton.h"
#include "ODBC.h"
