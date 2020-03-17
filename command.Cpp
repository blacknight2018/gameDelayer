#include "command.h"
void InitSocket()
{
	WSADATA Wsadata;
	WSAStartup(MAKEWORD(2, 2), &Wsadata);
}
SOCKET CreateTCPSocket()
{
	return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

SOCKET CreateUDPSocket()
{
	return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

bool BindPort(SOCKET serverSocket, SHORT bindPort)
{
	sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(bindPort);
	local.sin_addr.S_un.S_addr = INADDR_ANY;
	return 0==bind(serverSocket, (sockaddr*)&local, sizeof(sockaddr));
}