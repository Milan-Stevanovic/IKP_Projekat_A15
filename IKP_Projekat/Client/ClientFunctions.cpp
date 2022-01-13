#pragma once
#pragma comment (lib, "Ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define DEFAULT_BUFLEN 512
#include <winsock.h>
#include <stdio.h>
#include "../Common/Colors.h"

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
    printf("[ 1 ] Register ID\n");
    printf("[ 2 ] Send Data\n");
    printf("[ 3 ] Receive Data\n");\
    printf("[ 4 ] Relaunch Copy\n");
    printf("[ 5 ] Print your data\n");
    printf("[ 6 ] Stress Test (1000 messages)\n");
    printf("================================\n");
    int select = -1;
    do {
        printf("Select Option: ");
        scanf("%d", &select);
    } while (select < 1 || select > 6);

    return select;
}

SOCKET ConnectToServer(int port)
{
    SOCKET connectSocket = INVALID_SOCKET;

    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }

    connectSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(port);

    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
    }

    return connectSocket;
}

void SetSocketToNonBlockingMode(SOCKET socket)
{
    unsigned long mode = 1;
    ioctlsocket(socket, FIONBIO, &mode);
}

bool IsClientRegistered(int clientId)
{
    return clientId != 0 ? true : false;
}