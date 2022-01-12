#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define DEFAULT_BUFLEN 512

#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include "Colors.h"

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