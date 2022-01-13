#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "ReplicatorFunctions.h"
#include "../Common/ClientList.h"
#include "../Common/ResizableRingBuffer.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT_SERVER     "12000"
#define DEFAULT_PORT_CLIENT_COPY "13000"

HANDLE connClientCopies, msgClientCopies;
HANDLE msgServer;
HANDLE passServerMessageThread, passClientMessageToServerThread;
ClientNode* head = NULL;

char recvbuf[DEFAULT_BUFLEN];

RING_BUFFER ServerToClientRingBuffer;
RING_BUFFER ClientToServerRingBuffer;

int main()
{
    InitRing(&ServerToClientRingBuffer, RING_SIZE);
    InitRing(&ClientToServerRingBuffer, RING_SIZE);

    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }

    SOCKET serverListenSocket = SetUpListenSockets(DEFAULT_PORT_SERVER);
    SOCKET clientListenSocket = SetUpListenSockets(DEFAULT_PORT_CLIENT_COPY);
    int iResult;

    printf("Replicator initialized, waiting for server to connect.\n");

    // blocking function, waits till server connect to replicator
    SOCKET serverSocket = accept(serverListenSocket, NULL, NULL);
    if (serverSocket == INVALID_SOCKET)
    {
        printf("accept failed");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    else
    {
        printf("[ CONNECTION THREAD ] Server connected!\n");
    }

    SetSocketToNonBlockingMode(serverListenSocket);
    SetSocketToNonBlockingMode(clientListenSocket);
    
    // Thread
    connClientCopies = CreateThread(NULL, 0, &ConnectClientsThread, &clientListenSocket, NULL, NULL);
    msgClientCopies  = CreateThread(NULL, 0, &ClientMessageThread, NULL, NULL, NULL);
    msgServer        = CreateThread(NULL, 0, &ServerMessageThread, &serverSocket, NULL, NULL);
    passServerMessageThread = CreateThread(NULL, 0, &PassServerMessageThread, NULL, NULL, NULL);
    passClientMessageToServerThread = CreateThread(NULL, 0, &PassMessageFromClientToServer, &serverSocket, NULL, NULL);

    int lil = getchar();

    CloseHandle(connClientCopies);
    CloseHandle(msgClientCopies);
    CloseHandle(msgServer);
    CloseHandle(passServerMessageThread);
    CloseHandle(passClientMessageToServerThread);


    // Gracefull shutdown
    ClientNode* temp = head;
    while (temp != NULL)
    {
        iResult = shutdown(temp->acceptedSocket, SD_SEND);
        if (iResult == SOCKET_ERROR)
        {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(temp->acceptedSocket);
            WSACleanup();
            return 1;
        }
        temp = temp->next;
    }

    closesocket(serverListenSocket);
    closesocket(clientListenSocket);
    closesocket(serverSocket);

    WSACleanup();
    lil = getchar();
    return 0;
}

DWORD WINAPI ConnectClientsThread(LPVOID param) // Thread to connect CopyClients to replicator and to assign them accepted socket
{
    SOCKET* clientListenSocket = (SOCKET*)param;
    fd_set listenfds;
    FD_ZERO(&listenfds);

    timeval timeVal;
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;

    do
    {
        FD_SET(*clientListenSocket, &listenfds);

        int result = select(0, &listenfds, NULL, NULL, &timeVal);

        if (result == 0) {
            // vreme za cekanje je isteklo
        }
        else if (result == SOCKET_ERROR) {
            //desila se greska prilikom poziva funkcije
            // obrada error-a
        }
        else {
            if (FD_ISSET(*clientListenSocket, &listenfds))
            {
                SOCKET socket = accept(*clientListenSocket, NULL, NULL);
                if (socket == INVALID_SOCKET)
                {
                    printf("accept failed");
                    closesocket(socket);
                    WSACleanup();
                    return 1;
                }
                else
                {
                    AddClient(&head, socket, 0);
                    printf("[ CONNECTION THREAD ] Client connected!\n");
                }
            }
        }
    } while (1);
}

DWORD WINAPI ClientMessageThread(LPVOID param) // Recieve messages from CopyClients
{
    fd_set readfds;
    FD_ZERO(&readfds);

    timeval timeVal;
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;
    char recvbuf[DEFAULT_BUFLEN];
    Message* recievedMessageFromCopyClient;
    do
    {
        ClientNode* temp = head;
        while (temp != NULL)
        {
            if (temp->acceptedSocket != INVALID_SOCKET)
                FD_SET(temp->acceptedSocket, &readfds);
            temp = temp->next;
        }

        int result = select(0, &readfds, NULL, NULL, &timeVal);

        if (result == 0) {
            // vreme za cekanje je isteklo
        }
        else if (result == SOCKET_ERROR) {
            //desila se greska prilikom poziva funkcije
            // obrada error-a
        }
        else {
            temp = head;
            while (temp != NULL)
            {
                if (FD_ISSET(temp->acceptedSocket, &readfds))
                {
                    int iResult = recv(temp->acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);
                    if (iResult > 0)
                    {
                        recievedMessageFromCopyClient = (Message*)recvbuf;
                        if (temp->registerID == 0 && ntohs(recievedMessageFromCopyClient->flag) == REGISTER) // REGISTER
                        {
                            int id = atoi(recievedMessageFromCopyClient->data);
                            if (!ClientAlreadyRegistered(&head, id))
                            {
                                temp->registerID = id;
                                PrintList(&head);
                            }
                        }
                        if (temp->registerID != 0 && (ntohs(recievedMessageFromCopyClient->flag) == RECEIVE_DATA || ntohs(recievedMessageFromCopyClient->flag) == REQUEST_INTEGRITY_UPDATE)) // RECIEVE DATA
                        {
                            Push(&ClientToServerRingBuffer, recievedMessageFromCopyClient);
                            printf("R = %s\n", recievedMessageFromCopyClient->data);
                        }

                    }
                    else if (iResult == 0)
                    {
                        printf("Connection with clinet closed.\n");
                        closesocket(temp->acceptedSocket);
                        temp->acceptedSocket = INVALID_SOCKET;
                    }
                    else
                    {
                        printf(RED "Client Copy on SOCKET [ %d ] and ID [ %d ] closed.\n" WHITE, temp->acceptedSocket, temp->registerID);
                        ClientNode* next = temp->next;
                        temp->acceptedSocket = INVALID_SOCKET;
                        DeleteClientBySocket(&head, temp->acceptedSocket);
                        temp = head;
                        if (temp != NULL)
                        {
                            while (temp->next != next)
                            {
                                temp = temp->next;
                            }
                        }
                    }
                }
                if (temp != NULL)
                    temp = temp->next;
            }
        }
    } while (1);
}

DWORD WINAPI ServerMessageThread(LPVOID param) // Recieve messages from Server
{
    SOCKET* serverSocket = (SOCKET*)param;
    fd_set readfds;
    FD_ZERO(&readfds);

    timeval timeVal;
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;
    char recvbuf[DEFAULT_BUFLEN];
    Message* recievedMessageFromServer;

    while(true)
    {
        FD_SET(*serverSocket, &readfds);
        int result = select(0, &readfds, NULL, NULL, &timeVal);

        if (result == 0) {
            // vreme za cekanje je isteklo
        }
        else if (result == SOCKET_ERROR) {
            //desila se greska prilikom poziva funkcije
            // obrada error-a
        }
        else 
        {
            if (FD_ISSET(*serverSocket, &readfds))
            {
                int iResult = recv(*serverSocket, recvbuf, DEFAULT_BUFLEN, 0);
                if (iResult > 0)
                {
                    recievedMessageFromServer = (Message*)recvbuf;
                    Push(&ServerToClientRingBuffer, recievedMessageFromServer);
                    printf(YELLOW"\n [ RECIEVED SERVER MSG ] ID = %d | FLAG = %d | DATA = %s\n" WHITE,
                        ntohs(recievedMessageFromServer->id), ntohs(recievedMessageFromServer->flag), recievedMessageFromServer->data);
                }
            }
        }

    }
}

DWORD WINAPI PassServerMessageThread(LPVOID param)
{
    ClientNode* temp;
    struct Message passMessage;
    int iResult;

    while (true)
    {
        if (!IsEmpty(&ServerToClientRingBuffer))
        {
            passMessage = Pop(&ServerToClientRingBuffer);
            // Logic for processing message
            if (ntohs(passMessage.flag) == REGISTER || ntohs(passMessage.flag) == RELAUNCH_COPY) // REGISTER & RELAUNCH COPY
            {
                if (!ClientAlreadyRegistered(&head, atoi(passMessage.data)))
                {
                    LaunchClientCopy(atoi(passMessage.data));
                }
            }
            else if (ntohs(passMessage.flag) == SEND_DATA ||
                     ntohs(passMessage.flag) == RECEIVE_DATA ||
                     ntohs(passMessage.flag) == REQUEST_INTEGRITY_UPDATE) // SEND DATA & RECEIVE_DATA & REQUEST IU
            {
                if (!ClientAlreadyRegistered(&head, atoi(passMessage.data)))
                {
                    temp = head;
                    while (temp != NULL)
                    {
                        if (temp->registerID == ntohs(passMessage.id))
                        {
                            iResult = send(temp->acceptedSocket, (char*)&passMessage, sizeof(struct Message), 0);
                            if (iResult == SOCKET_ERROR)
                            {
                                printf(RED"Send failed with error: %d\n" WHITE, WSAGetLastError());
                            }
                        }
                        temp = temp->next;
                    }
                }
            }
        }
        else
        {
            Sleep(100);
        }
    }
}

DWORD WINAPI PassMessageFromClientToServer(LPVOID param)
{
    SOCKET* serverSocket = (SOCKET*)param;
    struct Message passMessage;
    int iResult;

    while (true)
    {
        if (!IsEmpty(&ClientToServerRingBuffer))
        {
            passMessage = Pop(&ClientToServerRingBuffer);

            iResult = send(*serverSocket, (char*)&passMessage, sizeof(struct Message), 0);
            if (iResult == SOCKET_ERROR)
            {
                printf(RED"Send failed with error: %d\n" WHITE, WSAGetLastError());
            }
        }
    }
}