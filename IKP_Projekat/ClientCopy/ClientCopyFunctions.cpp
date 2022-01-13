#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <direct.h>

bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}

int Menu()
{
    Sleep(100); // Giving time to RecieveMessageThread to print response if it has one
    printf("============= MENU =============\n");
    printf("[ 1 ] Receive Data\n");
    printf("[ 2 ] Request Integrity Update\n");
    printf("[ 3 ] Print your data\n");
    printf("================================\n");
    int select = -1;
    do {
        printf("Select Option: ");
        scanf("%d", &select);
    } while (select < 1 || select > 4);

    return select;
}

void SetSocketToNonBlockingMode(SOCKET socket)
{
    unsigned long mode = 1;
    ioctlsocket(socket, FIONBIO, &mode);
}

SOCKET ConnectCopyToReplicator(int port)
{
    SOCKET connectSocket = INVALID_SOCKET;

    connectSocket = socket(AF_INET,
                           SOCK_STREAM,
                           IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    sockaddr_in replicatorAddress;
    replicatorAddress.sin_family = AF_INET;
    replicatorAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    replicatorAddress.sin_port = htons(port);

    if (connect(connectSocket, (SOCKADDR*)&replicatorAddress, sizeof(replicatorAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
    }

    return connectSocket;
}