#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "ServerFunctions.h"
#include "../Common/ClientList.h"
#include "../Common/ResizableRingBuffer.h"

#define DEFAULT_BUFLEN 512
#define REPLICATOR_PORT 12000
#define CLIENT_PORT "11000"

HANDLE clientConnectHandle, clientMessageHandle, replicatorMessageHandle;
HANDLE passMessageHandle, passMessageFromReplicatorToClient;
ClientNode* head = NULL;

SOCKET replicatorSocket = INVALID_SOCKET;

RING_BUFFER ClientToReplicatorRingBuffer;
RING_BUFFER ReplicatorToClientRingBuffer;

int main()
{
    InitRing(&ClientToReplicatorRingBuffer, RING_SIZE);
    InitRing(&ReplicatorToClientRingBuffer, RING_SIZE);

    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }

    SOCKET listenSocket = SetUpListenSockets(CLIENT_PORT);

    
    SetSocketToNonBlockingMode(listenSocket);

    int iResult;
    printf("Server initialized, waiting for server to connect.\n");

    replicatorSocket = ReplicatorConnection(REPLICATOR_PORT);

    // Thread
    clientConnectHandle = CreateThread(NULL, 0, &ClientConnectionThread, &listenSocket, NULL, NULL);
    clientMessageHandle = CreateThread(NULL, 0, &ReceiveClientMessageThread, NULL, NULL, NULL);
    passMessageHandle   = CreateThread(NULL, 0, &PassMessageFromClientToReplicatorThread, &replicatorSocket, NULL, NULL);
    replicatorMessageHandle = CreateThread(NULL, 0, &ReceiveReplicatorMessageThread, &replicatorSocket, NULL, NULL);
    passMessageFromReplicatorToClient = CreateThread(NULL, 0, &PassMessageFromReplicatorToClientThread, NULL, NULL, NULL);

    int lil = getchar();

    printf(BLUE "\nPlease wait, clean up in progress ..." WHITE);
    
    // Gracefull shutdown
    CloseHandle(clientConnectHandle);
    CloseHandle(clientMessageHandle);
    CloseHandle(replicatorMessageHandle);
    CloseHandle(passMessageHandle);
    CloseHandle(passMessageFromReplicatorToClient);
    
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


    DeleteClientList(&head);
    DeleteRingBuffer(&ClientToReplicatorRingBuffer);
    DeleteRingBuffer(&ReplicatorToClientRingBuffer);

    closesocket(listenSocket);
    closesocket(replicatorSocket);

    WSACleanup();
    printf(BLUE "\nClean up is finished." WHITE);

    lil = getchar();

    return 0;
}

DWORD WINAPI ClientConnectionThread(LPVOID param)
{
    SOCKET* listenSocket = (SOCKET*)param;
    fd_set listenfds;
    FD_ZERO(&listenfds);

    timeval timeVal;
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;
    do
    {
        FD_SET(*listenSocket, &listenfds);

        int result = select(0, &listenfds, NULL, NULL, &timeVal);

        if (result == 0) {
            // vreme za cekanje je isteklo
        }
        else if (result == SOCKET_ERROR) {
            //desila se greska prilikom poziva funkcije
            // obrada error-a
        }
        else {
            if (FD_ISSET(*listenSocket, &listenfds))
            {
                SOCKET socket = accept(*listenSocket, NULL, NULL);
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
                    PrintList(&head);
                }
            }
        }
    } while (1);
}

DWORD WINAPI ReceiveClientMessageThread(LPVOID param)
{
    fd_set readfds;
    FD_ZERO(&readfds);

    timeval timeVal;
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;
    char recvbuf[DEFAULT_BUFLEN];
    Message* recievedMessageFromClients;

    while (true)
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
                        recievedMessageFromClients = (Message*)recvbuf;

                        Message response;
                        response.id = htons(recievedMessageFromClients->id);

                        switch (ntohs(recievedMessageFromClients->flag))
                        {
                            case REGISTER:
                                if (temp->registerID == 0)
                                {
                                    if (!ClientAlreadyRegistered(&head, atoi(recievedMessageFromClients->data)))
                                    {
                                        temp->registerID = atoi(recievedMessageFromClients->data);
                                        PrintList(&head);

                                        response.flag = htons(REGISTER);
                                        sprintf(response.data, "[ SUCCESS ] Registered with ID = %d", temp->registerID, strlen(response.data));

                                        recievedMessageFromClients->id = ntohs(temp->registerID);
                                        Push(&ClientToReplicatorRingBuffer, recievedMessageFromClients);
                                    }
                                    else
                                    {
                                        response.flag = htons(REGISTER);
                                        sprintf(response.data, "[ FAIL ] Try again, client with ID = %d already registered!", atoi(recievedMessageFromClients->data), strlen(response.data));
                                    }
                                }
                                // Send client response message to client
                                iResult = send(temp->acceptedSocket, (char*)&response, (int)sizeof(Message), 0);
                                break;
                            case SEND_DATA:
                                printf(YELLOW "[ MESSAGE THREAD ] Message from client %d: %s \n" WHITE, temp->registerID, recievedMessageFromClients->data);
                                Push(&ClientToReplicatorRingBuffer, recievedMessageFromClients);
                                sprintf(response.data, "[ SUCCESS ] Server recieved data! Data saved to client memory!", strlen(response.data));
                                break;
                            case REC_DATA:
                                printf(YELLOW "[ MESSAGE THREAD ] [ RECEIVE DATA ] Client %d sent data.\n" WHITE, temp->registerID);
                                Push(&ClientToReplicatorRingBuffer, recievedMessageFromClients);
                                break;
                            case INTEGRITY_UPDATE:
                                printf(YELLOW "[ MESSAGE THREAD ] [ INTEGRITY UPDATE ] Client %d sent data.\n" WHITE, temp->registerID);
                                Push(&ClientToReplicatorRingBuffer, recievedMessageFromClients);
                                break;
                            case REQ_REC_DATA:
                                printf(YELLOW "[ MESSAGE THREAD ] Client %d requested to recieve data.\n" WHITE, temp->registerID);
                                Push(&ClientToReplicatorRingBuffer, recievedMessageFromClients);
                                break;
                            case REQ_INTEGRITY_UPDATE:
                                printf(YELLOW "[ MESSAGE THREAD ] Client %d requested Integrity Update (IU).\n" WHITE, temp->registerID);
                                Push(&ClientToReplicatorRingBuffer, recievedMessageFromClients);
                                break;
                            case RELAUNCH_COPY:
                                printf(YELLOW "[ MESSAGE THREAD ] Client %d requested to relaunch client copy.\n" WHITE, temp->registerID);
                                Push(&ClientToReplicatorRingBuffer, recievedMessageFromClients);
                                break;
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
                        printf(RED "Client on SOCKET [ %d ] and ID [ %d ] closed.\n" WHITE, temp->acceptedSocket, temp->registerID);
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
    }
}

DWORD WINAPI PassMessageFromClientToReplicatorThread(LPVOID param)
{
    SOCKET* replicatorSocket = (SOCKET*)param;

    struct Message passMessage;
    int iResult;

    while (true)
    {
        if (!IsEmpty(&ClientToReplicatorRingBuffer))
        {
            passMessage = Pop(&ClientToReplicatorRingBuffer);
            iResult = send(*replicatorSocket, (char*)&passMessage, sizeof(struct Message), 0);
            if (iResult == SOCKET_ERROR)
            {
                printf(RED"Send failed with error: %d\n" WHITE, WSAGetLastError());
            }
        }
        else
        {
            Sleep(100);
        }
    }
}

DWORD WINAPI ReceiveReplicatorMessageThread(LPVOID param)
{
    SOCKET* replicatorSocket = (SOCKET*)param;
    fd_set readfds;
    FD_ZERO(&readfds);

    timeval timeVal;
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;
    char recvbuf[DEFAULT_BUFLEN];
    Message* recievedMessageFromServer;

    while (true)
    {
        FD_SET(*replicatorSocket, &readfds);
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
            if (FD_ISSET(*replicatorSocket, &readfds))
            {
                int iResult = recv(*replicatorSocket, recvbuf, sizeof(struct Message), 0);
                if (iResult > 0)
                {
                    recievedMessageFromServer = (Message*)recvbuf;
                    Push(&ReplicatorToClientRingBuffer, recievedMessageFromServer);
                    printf(YELLOW"\n [ RECIEVED REPLICATOR MSG ] ID = %d | FLAG = %d | DATA = %s\n" WHITE,
                        ntohs(recievedMessageFromServer->id), ntohs(recievedMessageFromServer->flag), recievedMessageFromServer->data);
                }
            }
        }

    }
}

DWORD WINAPI PassMessageFromReplicatorToClientThread(LPVOID param)
{
    ClientNode* temp;
    struct Message passMessage;
    int iResult;

    while (true)
    {
        if (!IsEmpty(&ReplicatorToClientRingBuffer))
        {
            passMessage = Pop(&ReplicatorToClientRingBuffer);
            // Logic for processing message
 
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
                Sleep(1);
            }
        }
        else
        {
            Sleep(10);
        }
    }
}