#pragma once
#pragma comment (lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string>
#include <direct.h>
#include "../Common/DataList.h";
#include "ClientCopyFunctions.h"
#define CLIENT_COPY_PORT 13000

int clientCopyId = -1;
DataNode* head = NULL;
HANDLE msgHandle;

bool InitializeWindowsSockets();
int Menu();

int main(int argc, char** argv)
{
    clientCopyId = atoi(argv[1]);
    printf("ClientCopy ID = %d\n", clientCopyId);

    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }

    SOCKET connectedSocket = ConnectCopyToReplicator(CLIENT_COPY_PORT);
    SetSocketToNonBlockingMode(connectedSocket);

    msgHandle = CreateThread(NULL, 0, &ReceiveReplicatorMessageThread, &connectedSocket, NULL, NULL);

    Message messageToSend;
    int iResult;

    // ClientCopy seniding his ID to replicator
    messageToSend.id = htons(clientCopyId);
    messageToSend.flag = htons(REGISTER);
    sprintf(messageToSend.data, "%d", clientCopyId, strlen(messageToSend.data));

    iResult = send(connectedSocket, (char*)&messageToSend, sizeof(struct Message), 0);

    if (iResult == SOCKET_ERROR)
    {
        printf("Send from ClientCopy failed with error: %d\n", WSAGetLastError());
        closesocket(connectedSocket);
        WSACleanup();
        return 1;
    }

    // Request Integrity Update on startup
    messageToSend.flag = htons(REQ_INTEGRITY_UPDATE);
    sprintf(messageToSend.data, "", strlen(messageToSend.data));
    iResult = send(connectedSocket, (char*)&messageToSend, (int)sizeof(Message), 0);

    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connectedSocket);
        WSACleanup();
        return 1;
    }


    bool sendMessage;
    while (true)
    {
        sendMessage = true;
        switch (Menu())
        {
            case 1: // Request Recieve Data
                messageToSend.id = htons(clientCopyId);
                messageToSend.flag = htons(REQ_REC_DATA);
                sprintf(messageToSend.data, "%d", clientCopyId, strlen(messageToSend.data));
                break;
            case 2: // Print Your Data
                PrintDataList(&head);
                sendMessage = false;
                break;
        }

        if (sendMessage)
        {
            iResult = send(connectedSocket, (char*)&messageToSend, (int)sizeof(Message), 0);

            if (iResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(connectedSocket);
                WSACleanup();
                return 1;
            }
        }
    }

    // Clean Up:
    printf(BLUE "\nPlease wait, clean up in progress ..." WHITE);
    CloseHandle(msgHandle);
    closesocket(connectedSocket);
    DeleteDataList(&head);
    WSACleanup();
    printf(BLUE "\nClean up is finished." WHITE);

    getchar();

    return 0;
}

DWORD WINAPI ReceiveReplicatorMessageThread(LPVOID param)
{
    Message* recievedMessage;
    char recvbuf[DEFAULT_BUFLEN];
    SOCKET* connectedSocket = (SOCKET*)param;

    fd_set readfds;
    FD_ZERO(&readfds);

    timeval timeVal;
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;
    int iResult;

    while (true)
    {
        FD_SET(*connectedSocket, &readfds);
        iResult = select(0, &readfds, NULL, NULL, &timeVal);

        if (iResult == 0) {
            // vreme za cekanje je isteklo
        }
        else if (iResult == SOCKET_ERROR) {
            printf(RED"[ ERROR ] %s\n" WHITE, WSAGetLastError());
        }
        else {
            if (FD_ISSET(*connectedSocket, &readfds))
            {
                // Recieve message from replicator
                iResult = recv(*connectedSocket, recvbuf, sizeof(struct Message), 0);
                if (iResult > 0)
                {
                    recievedMessage = (Message*)recvbuf;
                    switch (ntohs(recievedMessage->flag))
                    {
                        case SEND_DATA:
                            printf(MAGENTA "\n[ REPLICATOR MESSAGE ] [ SEND DATA ] %s\n" WHITE, recievedMessage->data);
                            PushData(&head, recievedMessage->data);
                            break;
                        case REC_DATA:
                            printf(MAGENTA "\n[ REPLICATOR MESSAGE ] [ RECEIVE DATA ] %s\n" WHITE, recievedMessage->data);
                            break;
                        case INTEGRITY_UPDATE:
                            printf(MAGENTA "\n[ REPLICATOR MESSAGE ] [ INTEGRITY UPDATE ] \n" WHITE);
                            PushData(&head, recievedMessage->data);
                            break;
                        case REQ_REC_DATA:
                            printf(MAGENTA "\n[ REPLICATOR MESSAGE ] [ REQUEST RECEIVE DATA ] Original requested to receive data.\n" WHITE);
                            ReceiveData(clientCopyId, &head, connectedSocket);
                            break;
                        case REQ_INTEGRITY_UPDATE:
                            printf(MAGENTA "\n[ REPLICATOR MESSAGE ] [ REQUEST INTEGRITY UPDATE ] Original requested Integrity Update (IU).\n" WHITE);
                            RequestIntegrityUpdate(clientCopyId, &head, connectedSocket);
                            break;
                    }
                }
                else if (iResult == 0)
                {
                    printf("Connection with clinet closed.\n");
                    closesocket(*connectedSocket);
                    *connectedSocket = INVALID_SOCKET;
                }
            }
        }
    }
}