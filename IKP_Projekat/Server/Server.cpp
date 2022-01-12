#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "ServerFunctions.h"
#include "../Common/ClientList.h"
#include "../Common/Message.h"

#define DEFAULT_BUFLEN 512
#define CLIENT_PORT "11000"
#define REPLICATOR_PORT 12000


ClientNode* head = NULL;
HANDLE clientConnectHandle, clientMessageHandle;


int main()
{
    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }

    SOCKET listenSocket = SetUpListenSockets(CLIENT_PORT);
    SetSocketToNonBlockingMode(listenSocket);

    int iResult;
    printf("Server initialized, waiting for server to connect.\n");

    // Thread
    clientConnectHandle = CreateThread(NULL, 0, &ClientConnectThread, &listenSocket, NULL, NULL);
    clientMessageHandle = CreateThread(NULL, 0, &ClientMessageThread, NULL, NULL, NULL);

    int lil = getchar();

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

    closesocket(listenSocket);

    CloseHandle(clientConnectHandle);
    CloseHandle(clientMessageHandle);

    WSACleanup();
    return 0;
}

DWORD WINAPI ClientConnectThread(LPVOID param)
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

DWORD WINAPI ClientMessageThread(LPVOID param)
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
                        response.id = ntohs(recievedMessageFromClients->id);

                        if (temp->registerID == 0 && ntohs(recievedMessageFromClients->flag) == REGISTER)
                        {
                            int id = ntohs(recievedMessageFromClients->id);
                            if (!ClientAlreadyRegistered(&head, atoi(recievedMessageFromClients->data)))
                            {
                                temp->registerID = atoi(recievedMessageFromClients->data);
                                PrintList(&head);

                                response.flag = htons(REGISTER);
                                sprintf(response.data, "[ SUCCESS ] Registered with ID = %d", temp->registerID, strlen(response.data));

                                

                                // Pass message to replicator
                                //iResult = send(replicatorSocket, recvbuf, DEFAULT_BUFLEN, 0);
                                //Push(&ringBuffer, (char *)recvbuf);
                            }
                            else
                            {
                                response.flag = htons(REGISTER);
                                sprintf(response.data, "[ FAIL ] Try again, client with ID = %d already registered!", id, strlen(response.data));

                            }
                        }
                        else if (ntohs(recievedMessageFromClients->flag) == SEND_DATA)
                        {
                            printf(YELLOW "[ MESSAGE THREAD ] Message from client %d: %s \n" WHITE, temp->registerID, recievedMessageFromClients->data);

                            //Push(&ringBuffer, (char *)recvbuf);
                            // Pass message to replicator
                            //iResult = send(replicatorSocket, recvbuf, DEFAULT_BUFLEN, 0);

                            sprintf(response.data, "[ SUCCESS ] Server recieved data! Data saved to client memory!", strlen(response.data));
                        }
                        /*else if (strcmp(recievedStruct->flag, "[rlc]") == 0)
                        {
                            printf(YELLOW "[ MESSAGE THREAD ] Request from client to relaunch client copy %d \n" WHITE, temp->registerID);

                            //Push(&ringBuffer, (char*)recvbuf);

                            // Pass message to replicator
                            //sprintf(messageToSend, "[rlc]%d", temp->registerID, strlen(messageToSend));
                            //iResult = send(replicatorSocket, recvbuf, DEFAULT_BUFLEN, 0);

                            sprintf(messageToSend, "[ SUCCESS ] Request passed to replicator! Starting client copy soon.", strlen(messageToSend));
                        }
                        */
                        // Send client response message to client
                        iResult = send(temp->acceptedSocket, (char*)&response, (int)sizeof(Message), 0);
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