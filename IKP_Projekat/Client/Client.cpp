#define WIN32_LEAN_AND_MEAN
#include "../Common/DataList.h"
#include "ClientFunctions.h"

#define DEFAULT_PORT 11000

#pragma region GlobalVariables
int clientID = 0;
DataNode* head = NULL;
HANDLE msgHandle;
#pragma endregion GlobalVariables

int main()
{
	int iResult;
	Message message;

	SOCKET connectedSocket = ConnectToServer(DEFAULT_PORT);
	SetSocketToNonBlockingMode(connectedSocket);

	msgHandle = CreateThread(NULL, 0, &RecieveMessageFromServerThread, &connectedSocket, NULL, NULL);

    bool sendMessage;
    while (true)
    {
        sendMessage = true;
        switch (Menu())
        {
            case 1: // Register
                if (!IsClientRegistered(clientID))
                {
                    do {
                        printf("Enter your ID:");
                        scanf("%d", &clientID);
                    } while (clientID < 1);

                    // message to send
                    message.id = htons(0);
                    message.flag = htons(REGISTER);
                    sprintf(message.data, "%d", clientID, strlen(message.data));
                }
                else
                {
                    sendMessage = false;
                }
                break;

            case 2: // Send data
                if (IsClientRegistered(clientID))
                {
                    printf("Enter message: ");
                    getchar();
                    char data[DEFAULT_BUFLEN];
                    gets_s(data, DEFAULT_BUFLEN);

                    message.id = htons(clientID);
                    message.flag = htons(SEND_DATA);
                    sprintf(message.data, "%s", data, strlen(message.data));
                    PushData(&head, data);
                }
                else
                {
                    sendMessage = false;
                }
                break;
            case 3: // Recieve Data
                if (IsClientRegistered(clientID))
                {
                    message.id = htons(clientID);
                    message.flag = htons(RECEIVE_DATA);
                    sprintf(message.data, "%s", "RECEIVE DATA", strlen(message.data));
                }
                else
                {
                    sendMessage = false;
                }
                break;
            case 4: // Request Integrity Update
                if (IsClientRegistered(clientID))
                {
                    message.id = htons(clientID);
                    message.flag = htons(REQUEST_INTEGRITY_UPDATE);
                    sprintf(message.data, "%s", "REQUEST INTEGRITY UPDATE", strlen(message.data));
                }
                else
                {
                    sendMessage = false;
                }
                break;
            case 5: // Relaunch Copy Client
                if (IsClientRegistered(clientID))
                {
                    message.id = htons(clientID);
                    message.flag = htons(RELAUNCH_COPY);
                    sprintf(message.data, "%d", clientID, strlen(message.data));
                }
                else
                {
                    sendMessage = false;
                }
                break;
            case 6: // Print Data
                if (IsClientRegistered(clientID))
                {
                    PrintDataList(&head);
                }
                sendMessage = false;
                break;
        }

        if (sendMessage)
        {
            iResult = send(connectedSocket, (char*)&message, (int)sizeof(Message), 0);

            if (iResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(connectedSocket);
                WSACleanup();
                return 1;
            }
        }
    }

    CloseHandle(msgHandle);

    closesocket(connectedSocket);
    WSACleanup();

    return 0;
}

DWORD WINAPI RecieveMessageFromServerThread(LPVOID param)
{
    Message *recievedMessage;
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
                // Recieve response from server
                iResult = recv(*connectedSocket, recvbuf, sizeof(struct Message), 0);
                if (iResult > 0)
                {
                    recievedMessage = (Message*)recvbuf;
                    printf(MAGENTA "\n[ SERVER MESSAGE ] %s\n\n" WHITE, recievedMessage->data);
                    // TODO: Check flags and add logic
                    char compareString[256];
                    sprintf(compareString, "[ FAIL ] Try again, client with ID = %d already registered!", clientID, strlen(compareString));
                    if (strcmp(recievedMessage->data, compareString) == 0)
                        clientID = 0;
                }
                else if (iResult == 0)
                {
                    printf("Connection with clinet closed.\n");
                    closesocket(*connectedSocket);
                    *connectedSocket = INVALID_SOCKET;
                }
            }
        }
        Sleep(100);
    }
}