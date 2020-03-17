#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <map>
#include <vector>
#include <queue>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

constexpr auto PROXYPORT = 1080;
constexpr auto UDPBIND   = 10000;

void InitSocket();

bool BindPort(SOCKET serverSocket,SHORT bindPort);

SOCKET CreateTCPSocket();
SOCKET CreateUDPSocket();
