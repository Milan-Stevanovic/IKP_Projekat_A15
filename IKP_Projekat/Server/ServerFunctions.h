/*
	Function: InitializeWindowsSockets
	------------------------------------
	[ Functionality ]: Initialize WsaData
	[     Params    ]: None
	[  Return Value ]: success of the action -> bool
*/
bool InitializeWindowsSockets();

/*
	Function: ReplicatorConnection
	------------------------------------
	[ Functionality ]: 
	[     Params    ]: 
	[  Return Value ]: 
*/
SOCKET ReplicatorConnection();

/*
	Function: SetSocketToNonBlockingMode
	------------------------------------
	[ Functionality ]: Set socket to non blocking mode
	[     Params    ]: socket -> SOCKET
	[  Return Value ]: None
*/
void SetSocketToNonBlockingMode(SOCKET socket);

/*
	Function: ClientConnectThread
	------------------------------------
	[ Functionality ]:
	[     Params    ]:
	[  Return Value ]:
*/
DWORD WINAPI ClientConnectThread(LPVOID param);

/*
	Function: ClientMessageThread
	------------------------------------
	[ Functionality ]:
	[     Params    ]:
	[  Return Value ]:
*/
DWORD WINAPI ClientMessageThread(LPVOID param);

/*
	Function: SetUpListenSockets
	------------------------------------
	[ Functionality ]:
	[     Params    ]:
	[  Return Value ]:
*/
SOCKET SetUpListenSockets(const char* port);