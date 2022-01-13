#pragma once
#pragma comment (lib, "Ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define DEFAULT_BUFLEN 512
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
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

SOCKET ReplicatorConnection(int port)
{
    SOCKET replicatorSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (replicatorSocket == INVALID_SOCKET)
    {
        printf("Replicator Socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    sockaddr_in replicatorAddress;
    replicatorAddress.sin_family = AF_INET;
    replicatorAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    replicatorAddress.sin_port = htons(port);

    if (connect(replicatorSocket, (SOCKADDR*)&replicatorAddress, sizeof(replicatorAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to replicator.\n");
        closesocket(replicatorSocket);
        WSACleanup();
    }
    return replicatorSocket;
}

void SetSocketToNonBlockingMode(SOCKET socket)
{
    unsigned long mode = 1;
    ioctlsocket(socket, FIONBIO, &mode);
}

SOCKET SetUpListenSockets(const char* port)
{
    SOCKET listenSocket = INVALID_SOCKET;

    addrinfo* resultingAddress = NULL;

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int iResult = getaddrinfo(NULL, port, &hints, &resultingAddress);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    listenSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (listenSocket == INVALID_SOCKET)
    {
        printf("serverListenSocket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
        return 1;
    }

    iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    return listenSocket;
}