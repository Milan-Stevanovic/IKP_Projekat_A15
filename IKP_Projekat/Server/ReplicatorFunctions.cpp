#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <direct.h>
#define DEFAULT_BUFLEN 512

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

void LaunchClientCopy(int clientId)
{
    char command[DEFAULT_BUFLEN];
    _chdir("../x64/Debug/");
    sprintf(command, "start ClientCopy.exe %d", clientId, DEFAULT_BUFLEN);
    (void)system(command);
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

void SetSocketToNonBlockingMode(SOCKET socket)
{
    unsigned long mode = 1;
    ioctlsocket(socket, FIONBIO, &mode);
}