#include "udpTransfer.h"
#define MAXBUFFERSIZE 5024


CRITICAL_SECTION csc;
std::queue< sendtoPacked > sendtoProxyClient;
std::queue< sendtoPacked > sendtoRemoteServer;

std::vector<std::pair<SOCKET, SOCKET> >groupSocket;


void CleanSocket(SOCKET udpSocket)
{
	EnterCriticalSection(&csc);
	for (auto& d : groupSocket) {
		if (d.first == udpSocket) {
			closesocket(d.first);
			closesocket(d.second);
			break;
		}
	}
	LeaveCriticalSection(&csc);
}
void TransferThread()
{
	int recvlen = 0,sendlen = 0,frontlen = 0;
	char front[100] = { 0 };
	char buffer[MAXBUFFERSIZE] = { 0 }, buffer_remote[MAXBUFFERSIZE] = { 0 };
	USHORT dstport = 0;
	in_addr dstaddr = { 0 };
	sockaddr_in client = { 0 };
	while (true) {
		int len = groupSocket.size();
		//for (std::pair<SOCKET, SOCKET>&d : groupSocket) {
		for (int i = 0; i < len;i++) {
			SOCKET serudpSocket = groupSocket[i].first;
			SOCKET remoteSocket = groupSocket[i].second;
			if (serudpSocket && remoteSocket) {
				/* Recv Data From local Client */
				int addrlen = sizeof(sockaddr_in);
				recvlen = recvfrom(serudpSocket, buffer, MAXBUFFERSIZE, 0, (sockaddr*)&client, &addrlen);
				if (recvlen > 5 && buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 0) {
					//ipv4
					if (buffer[3] == 1) {
						//
						dstaddr.S_un.S_addr = *(long*)&buffer[4];
						dstport = *(USHORT*)&buffer[8];
						dstport = ntohs(dstport);
						//std::cout << inet_ntoa(dstaddr) << ":" << dstport << std::endl;
						
						//IPV4+PORT+4 = 10
						frontlen = 10;
						memcpy(front, &buffer[0], frontlen);

						//
						sockaddr_in remoteaddr = { 0 };
						remoteaddr.sin_family = AF_INET;
						remoteaddr.sin_port = htons(dstport);
						remoteaddr.sin_addr = dstaddr;
						//sendlen = sendto(remoteSocket, &buffer[10], recvlen - 10, 0, (sockaddr*)&remoteaddr, sizeof(sockaddr_in));
						sendtoPacked p;
						p.addr = remoteaddr;
						p.s = remoteSocket;
						p.ptrData = (PBYTE)new char[recvlen - 10];
						p.dataLen = recvlen - 10;
						memcpy(p.ptrData, &buffer[10], recvlen - 10);
						EnterCriticalSection(&csc);
						sendtoRemoteServer.push(p);
						LeaveCriticalSection(&csc);


					}
					//domain
					else if (buffer[3] > 1) {

					}
				}

				/* Recv from Remote Server */

				memcpy(buffer_remote, front,frontlen);

				sockaddr_in localremotaddr;
				recvlen = recvfrom(remoteSocket, &buffer_remote[frontlen], MAXBUFFERSIZE, 0, (sockaddr*)&localremotaddr, &addrlen);
				if (recvlen > 0)
				{
					//Delay Position
					//sendlen = sendto(serudpSocket, buffer_remote, frontlen + recvlen, 0,(sockaddr*)&client, sizeof(sockaddr_in));
					sendtoPacked p;
					p.addr = client;
					p.s = serudpSocket;
					p.ptrData = (PBYTE)new char[frontlen + recvlen];
					p.dataLen = frontlen + recvlen;
					memcpy(p.ptrData, &buffer_remote[0], frontlen + recvlen);
					EnterCriticalSection(&csc);
					sendtoProxyClient.push(p);
					LeaveCriticalSection(&csc);

					
				}


			}
		}
		Sleep(1);
	}
	
}

void SendQueue()
{
	while (true) {
		while (!sendtoRemoteServer.empty())
		{
			sendtoPacked& t = sendtoRemoteServer.front();
			sendto(t.s, (const char*)t.ptrData, t.dataLen, 0,(const sockaddr*) &t.addr, sizeof(sockaddr));
			delete []t.ptrData;
			EnterCriticalSection(&csc);
			sendtoRemoteServer.pop();
			LeaveCriticalSection(&csc);
			//std::cout << "Send" << std::endl;
		}
		Sleep(500);
		
	}
}


void RecvQueue()
{
	while (true) {
		while (!sendtoProxyClient.empty())
		{
			sendtoPacked& t = sendtoProxyClient.front();
			sendto(t.s, (const char*)t.ptrData, t.dataLen, 0, (const sockaddr*)&t.addr, sizeof(sockaddr));
			delete[]t.ptrData;
			EnterCriticalSection(&csc);
			sendtoProxyClient.pop();
			LeaveCriticalSection(&csc);
			//std::cout << "Recv" << std::endl;
		}
		Sleep(10);
	}
}
bool StartUDPServer()
{

	InitializeCriticalSection(&csc);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)TransferThread, NULL, NULL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SendQueue, NULL, NULL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)RecvQueue, NULL, NULL, NULL);
	
	return false;
}

USHORT getUdpPort()
{
	USHORT localPort = 0;
	SOCKET recvClient = CreateUDPSocket();
	SOCKET recvRemote = CreateUDPSocket();

	//
	int len = sizeof(sockaddr_in);
	sockaddr_in local;
	
	BindPort(recvClient, 0);
	getsockname(recvClient, (sockaddr*)&local, &len);
	localPort = ntohs(local.sin_port);

	
	BindPort(recvRemote, 0);

	unsigned long ul = 1;
	ioctlsocket(recvClient, FIONBIO, &ul);
	ioctlsocket(recvRemote, FIONBIO, &ul);


	groupSocket.push_back(std::make_pair(recvClient, recvRemote));
	return localPort;
}