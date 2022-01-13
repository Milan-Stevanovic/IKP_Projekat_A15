/*
	Function: 
	------------------------------------
	[ Functionality ]: 
	[     Params    ]: 
	[  Return Value ]: 
*/
bool InitializeWindowsSockets();

/*
	Function:
	------------------------------------
	[ Functionality ]:
	[     Params    ]:
	[  Return Value ]:
*/
int Menu();

/*
	Function:
	------------------------------------
	[ Functionality ]:
	[     Params    ]:
	[  Return Value ]:
*/
SOCKET ConnectCopyToReplicator(int port);

/*
	Function: SetSocketToNonBlockingMode
	------------------------------------
	[ Functionality ]: Set socket to non blocking mode
	[     Params    ]: socket -> SOCKET
	[  Return Value ]: None
*/
void SetSocketToNonBlockingMode(SOCKET socket);

/*
	Function: RecieveMessageFromReplicatorThread
	------------------------------------
	[ Functionality ]: Thread that is used to receive messages from replicator
	[     Params    ]: socket -> SOCKET*
	[  Return Value ]: None
*/
DWORD WINAPI RecieveMessageFromReplicatorThread(LPVOID param);
