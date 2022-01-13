#pragma once
#pragma comment (lib, "Ws2_32.lib")

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "Message.h"

#define DEFAULT_BUFLEN 512
#define RING_SIZE 4

typedef struct RingBuffer {
	int head, tail;
	int size;
	struct Message* data;
	CRITICAL_SECTION cs;
}RING_BUFFER;

/*
	Function: InitRing
	------------------------------------
	[ Functionality ]: Initialize starting parameters for ring (queue)
	[     Params    ]: ring -> RING_BUFFER*, size -> int
	[  Return Value ]: None
*/
void InitRing(RING_BUFFER* ring, int size);

/*
	Function: Push
	------------------------------------
	[ Functionality ]: Insert data to queue
	[     Params    ]: ring    -> RING_BUFFER*
					   newData -> struct Message*
	[  Return Value ]: None
*/
void Push(RING_BUFFER* ring, struct Message* newData);

/*
	Function: Pop
	------------------------------------
	[ Functionality ]: Get data at 'head' index and delete it from queue
	[     Params    ]: ring -> RING_BUFFER*
	[  Return Value ]: data -> struct Message
*/
struct Message Pop(RING_BUFFER* ring);

/*
	Function: DisplayRing
	------------------------------------
	[ Functionality ]: Print all elements in the queue
	[     Params    ]: ring -> RING_BUFFER*
	[  Return Value ]: None
*/
void DisplayRing(RING_BUFFER* ring);

/*
	Function: ResizeQueue
	------------------------------------
	[ Functionality ]: Doubles the queue size if queue is full and Push method is called
	[     Params    ]: ring -> RING_BUFFER*
	[  Return Value ]: None
*/
void ResizeQueue(RING_BUFFER* ring);

/*
	Function: IsEmpty
	------------------------------------
	[ Functionality ]: Chek if buffer is empty
	[     Params    ]: ring -> RING_BUFFER*
	[  Return Value ]: bool
*/
bool IsEmpty(RING_BUFFER* ring);


void InitRing(RING_BUFFER* ring, int size)
{
	ring->head = -1;
	ring->tail = -1;
	ring->size = size;
	InitializeCriticalSection(&(ring->cs));
	ring->data = (struct Message*)malloc(ring->size * sizeof(struct Message));
}

void Push(RING_BUFFER* ring, struct Message* newData)
{
	EnterCriticalSection(&(ring->cs));
	if ((ring->head == 0 && ring->tail == ring->size - 1) || (ring->tail == (ring->head - 1) % (ring->size - 1)))
	{
		LeaveCriticalSection(&(ring->cs));
		ResizeQueue(ring);
		Push(ring, newData);
	}
	else if (ring->head == -1)
	{
		ring->head = 0;
		ring->tail = 0;
		ring->data[ring->tail] = *newData;
	}
	else if (ring->tail == ring->size - 1 && ring->head != 0)
	{
		ring->tail = 0;
		ring->data[ring->tail] = *newData;
	}
	else
	{
		ring->tail++;
		ring->data[ring->tail] = *newData;
	}
	LeaveCriticalSection(&(ring->cs));
}

bool IsEmpty(RING_BUFFER* ring)
{
	EnterCriticalSection(&(ring->cs));
	if (ring->head == -1)
	{
		LeaveCriticalSection(&(ring->cs));
		return true;
	}
	LeaveCriticalSection(&(ring->cs));
	return false;
}

struct Message Pop(RING_BUFFER* ring)
{
	struct Message data;
	data.id = (ring->data[ring->head]).id;
	data.flag = (ring->data[ring->head]).flag;
	strcpy(data.data, (ring->data[ring->head]).data);

	if (ring->head == ring->tail)
	{
		ring->head = -1;
		ring->tail = -1;
		ring->size = RING_SIZE;
		free(ring->data);
		ring->data = (struct Message*)malloc(ring->size * sizeof(struct Message));
	}
	else if (ring->head == ring->size - 1)
	{
		ring->head = 0;
	}
	else
	{
		ring->head++;
	}
	LeaveCriticalSection(&(ring->cs));

	//printf("POPED: %s\n", data.data);
	return data;
}

void DisplayRing(RING_BUFFER* ring)
{
	EnterCriticalSection(&(ring->cs));
	if (ring->head == -1)
	{
		LeaveCriticalSection(&(ring->cs));
		printf("\nRingSize = %d\n", ring->size);
		printf("\n[ RING BUFFER EMPTY ]\n");
		return;
	}

	printf("\nRingSize = %d\n", ring->size);
	if (ring->tail >= ring->head)
	{
		for (int i = ring->head; i <= ring->tail; i++)
			printf("%s ", ring->data[i].data);
	}
	else
	{
		for (int i = ring->head; i < ring->size; i++)
			printf("%s ", ring->data[i].data);

		for (int i = 0; i <= ring->tail; i++)
			printf("%s ", ring->data[i].data);
	}
	LeaveCriticalSection(&(ring->cs));
}

void ResizeQueue(RING_BUFFER* ring)
{
	EnterCriticalSection(&(ring->cs));
	int size = ring->size;
	ring->size *= 2;
	ring->data = (struct Message*)realloc(ring->data, ring->size * sizeof(struct Message));
	if (!ring->data)
	{
		LeaveCriticalSection(&(ring->cs));
		printf("Memory error!\n");
		return;
	}

	if (ring->head > ring->tail)
	{
		for (int i = 0; i < ring->head; i++)
		{
			ring->data[i + size] = ring->data[i];
			//ring->data[i] = NULL;
		}
		ring->tail += size;
	}
	LeaveCriticalSection(&(ring->cs));
}