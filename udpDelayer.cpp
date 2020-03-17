#include "command.h"
#include "udpTransfer.h"
SOCKET g_serSocket = 0;
bool StartServer()
{
    g_serSocket = CreateTCPSocket();
    BindPort(g_serSocket, PROXYPORT);
    return listen(g_serSocket, 0);
}
void HandleThread(SOCKET clientSocket)
{
    int recvlen = 0,sendlen = 0 ;
    char buffer[100] = { 0 };
    
    recvlen = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (recvlen && recvlen==3 && buffer[0] == 5 && buffer[2] == 0) {
        buffer[0] = 5, buffer[1] = 0;
        sendlen = send(clientSocket, buffer, 2, 0);
        if (sendlen == 2) {
            recvlen = recv(clientSocket, buffer, 4, 0);

            int addrtype = 0;
            if (buffer[0] == 5 && buffer[1] == 3 && buffer[2] == 0) {
                buffer[0] = 5, buffer[1] = buffer[2] = 0, buffer[3] = 1;
                buffer[4] = buffer[5] = buffer[6] = buffer[7] = 0;
                USHORT allocport = getUdpPort();
                *((USHORT*)&buffer[8]) = htons(allocport);
                sendlen = send(clientSocket, buffer, 10, 0);
                if (sendlen) {
                    do {
                        recvlen = recv(clientSocket, buffer, 1, 0);
                    } while (recvlen);
                }
                CleanSocket(allocport);
            }
        }

    }

    closesocket(clientSocket);
}
void AuthClient()
{
    while (g_serSocket) {
        int addrLen = sizeof(sockaddr);
        sockaddr clientAddr;
        SOCKET clientSocket = accept(g_serSocket, &clientAddr, &addrLen);
        if (clientSocket) {
            CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)HandleThread, (LPVOID)clientSocket, NULL, nullptr);
        }
    }
}
int main()
{
    InitSocket();
    StartUDPServer();
    StartServer();
    AuthClient();

    return 0;
}