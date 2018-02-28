#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")

#include <iostream>
#include <thread>
#include <WinSock2.h>
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <thread>

#define MAX_USER 10

#include "User.h"
#include "ChattingServer.h"


using namespace std;
