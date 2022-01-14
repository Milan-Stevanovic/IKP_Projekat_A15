#pragma once
#include <ws2tcpip.h>
#include "Colors.h";
#include <stdio.h>

/*
    Function: AddClient
    ------------------------------------
    [ Functionality ]: Add client to linked list
    [     Params    ]: head -> ClientNode**
                       acceptedSocket -> SOCKET
                       registerID -> int
    [  Return Value ]: None
*/
void AddClient(struct ClientNode** head, SOCKET acceptedSocket, int registerID);

/*
    Function: DeleteClientBySocket
    ------------------------------------
    [ Functionality ]: Delete client from linked list by his unique accepted socket
    [     Params    ]: head -> ClientNode**
                       socket -> SOCKET
    [  Return Value ]: None
*/
void DeleteClientBySocket(struct ClientNode** head, SOCKET socket);

/*
    Function: PrintList
    ------------------------------------
    [ Functionality ]: Print all elements in clints linked list
    [     Params    ]: head -> ClientNode**
    [  Return Value ]: None
*/
void PrintList(struct ClientNode** head);

/*
    Function: ClientAlreadyRegistered
    ------------------------------------
    [ Functionality ]: Check if client with the same ID is already registered
    [     Params    ]: head -> ClientNode**
                       id -> int
    [  Return Value ]: bool
*/
bool ClientAlreadyRegistered(struct ClientNode** head, int id);

/*
    Function: DeleteClientList
    ------------------------------------
    [ Functionality ]: Delets all elements of the list and initialize head to NULL
    [     Params    ]: head -> struct ClientNode**
    [  Return Value ]: None
*/
void DeleteClientList(struct ClientNode** head);


struct ClientNode {
    SOCKET acceptedSocket;
    int registerID;
    struct ClientNode* next;
};

void AddClient(struct ClientNode** head, SOCKET acceptedSocket, int registerID)
{
    struct ClientNode* newNode = (struct ClientNode*)malloc(sizeof(struct ClientNode));
    newNode->acceptedSocket = acceptedSocket;
    newNode->registerID = registerID;
    newNode->next = NULL;

    if (*head == NULL)
        *head = newNode;
    else
    {
        struct ClientNode* lastNode = *head;

        while (lastNode->next != NULL)
        {
            lastNode = lastNode->next;
        }

        lastNode->next = newNode;
    }
}

void DeleteClientBySocket(struct ClientNode** head, SOCKET socket)
{
    ClientNode* temp = *head;
    ClientNode* prev = *head;

    if (temp != NULL && temp->acceptedSocket == socket) {
        *head = temp->next;
        closesocket(temp->acceptedSocket);
        free(temp);
        return;
    }

    while (temp != NULL && temp->acceptedSocket != socket) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL)
        return;

    prev->next = temp->next;
    closesocket(temp->acceptedSocket);
    free(temp);
}

void PrintList(struct ClientNode** head)
{
    printf(GREEN"============ CLIENT LIST ============\n");
    struct ClientNode* temp = *head;
    int i = 1;
    while (temp != NULL)
    {
        printf("[No. %*d] | [Socket = %d] | [ID = %d]\n", 2, i++, temp->acceptedSocket, temp->registerID);
        temp = temp->next;
    }

    printf("=====================================\n" WHITE);
}

bool ClientAlreadyRegistered(struct ClientNode** head, int id)
{
    struct ClientNode* temp = *head;
    while (temp != NULL)
    {
        if (temp->registerID == id)
        {
            return true;
        }
        temp = temp->next;
    }
    return false;
}

void DeleteClientList(struct ClientNode** head)
{
    struct ClientNode* current = *head;
    struct ClientNode* next = NULL;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

    *head = NULL;
}