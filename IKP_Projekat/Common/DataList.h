#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define DEFAULT_BUFLEN 512

#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include "../Common/Message.h"
#include "Colors.h"

CRITICAL_SECTION lock;

/*
    Function: PushData
    ------------------------------------
    [ Functionality ]: Store date on client local storage (linked list)
    [     Params    ]: head -> DataNode**
                       data -> char*
    [  Return Value ]: None
*/
void PushData(struct DataNode** head, char* data);

/*
    Function: PrintDataList
    ------------------------------------
    [ Functionality ]: Print all elements in linked list on client side
    [     Params    ]: head -> DataNode**
    [  Return Value ]: None
*/
void PrintDataList(struct DataNode** head);

/*
    Function: DeleteDataList
    ------------------------------------
    [ Functionality ]: Delets all elements of the list and initialize head to NULL
    [     Params    ]: head -> struct DataNode**
    [  Return Value ]: None
*/
void DeleteDataList(struct DataNode** head);

/*
    Function: ReceiveData
    ------------------------------------
    [ Functionality ]: Send all data to original
    [     Params    ]: clientID -> int, head -> DataNode*, socket -> SOCKET*
    [  Return Value ]: None
*/
void ReceiveData(int clientID, DataNode** head, SOCKET* socket);

/*
    Function: RequestIntegrityUpdate
    ------------------------------------
    [ Functionality ]: Send all data to original
    [     Params    ]: clientID -> int, head -> DataNode*, socket -> SOCKET*
    [  Return Value ]: None
*/
void RequestIntegrityUpdate(int clientID, DataNode** head, SOCKET* socket);

struct DataNode {
    char data[DEFAULT_BUFLEN];
    struct DataNode* next;
};

void PushData(struct DataNode** head, char* data)
{
    struct DataNode* newNode = (struct DataNode*)malloc(sizeof(struct DataNode));
    newNode->next = NULL;
    strcpy(newNode->data, data);

    if (*head == NULL)
        *head = newNode;
    else
    {
        struct DataNode* lastNode = *head;

        while (lastNode->next != NULL)
        {
            lastNode = lastNode->next;
        }

        lastNode->next = newNode;
    }
}


void PrintDataList(struct DataNode** head)
{
    printf(BLUE "\n============= DATA LIST =============\n");
    struct DataNode* temp = *head;
    int i = 1;
    while (temp != NULL)
    {
        printf("[No. %*d] | [Data]: %s\n", 2, i++, temp->data);
        temp = temp->next;
    }
    printf("=====================================\n\n" WHITE);
}

void DeleteDataList(struct DataNode** head)
{
    struct DataNode* current = *head;
    struct DataNode* next = NULL;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

    *head = NULL;
}

void ReceiveData(int clientID, DataNode** head, SOCKET* socket)
{
    struct DataNode* temp = *head;
    Message messageToSend;
    int iResult;
    messageToSend.id = htons(clientID);
    messageToSend.flag = htons(REC_DATA);

    while (temp != NULL)
    {
        strcpy(messageToSend.data, temp->data);
        iResult = send(*socket, (char*)&messageToSend, (int)sizeof(Message), 0);

        if (iResult == SOCKET_ERROR)
        {
            printf(RED"RECEIVE DATA failed to send message: %d\n" WHITE, WSAGetLastError());
        }
        temp = temp->next;
        Sleep(10); // Give time to Replicator to read all messages
    }
}

void RequestIntegrityUpdate(int clientID, DataNode** head, SOCKET* socket)
{
    struct DataNode* temp = *head;
    Message messageToSend;
    int iResult;
    messageToSend.id = htons(clientID);
    messageToSend.flag = htons(INTEGRITY_UPDATE);

    while (temp != NULL)
    {
        strcpy(messageToSend.data, temp->data);
        iResult = send(*socket, (char*)&messageToSend, (int)sizeof(Message), 0);

        if (iResult == SOCKET_ERROR)
        {
            printf(RED"RECEIVE DATA failed to send message: %d\n" WHITE, WSAGetLastError());
        }
        temp = temp->next;
        Sleep(10); // Give time to Replicator to read all messages
    }
}