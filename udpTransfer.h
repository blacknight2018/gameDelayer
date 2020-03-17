#pragma once
#include "command.h"

struct sendtoPacked {
	SOCKET s;
	PBYTE ptrData;
	LONG  dataLen;
	sockaddr_in addr;
};

void CleanSocket(SOCKET udpSocket);

bool StartUDPServer();

USHORT getUdpPort();