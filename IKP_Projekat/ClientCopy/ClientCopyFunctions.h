/*
	Function: InitializeWindowsSockets
	------------------------------------
	[ Functionality ]: Setd up WsaData values
	[     Params    ]: None
	[  Return Value ]: bool 
*/
bool InitializeWindowsSockets();

/*
	Function: Menu
	------------------------------------
	[ Functionality ]: Prints all available options and reads client response.
	[     Params    ]: None
	[  Return Value ]: option selected -> int
*/
int Menu();

/*
	Function: ConnectCopyToReplicator
	------------------------------------
	[ Functionality ]: Makes new SOCKET, sets all necessary values and connects socket to Server base on port
	[     Params    ]: (Servers) port -> int
	[  Return Value ]: SOCKET connectedSocket
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
	Function: ReceiveReplicatorMessageThread
	------------------------------------
	[ Functionality ]: Thread that is used to receive messages from replicator
	[     Params    ]: socket -> SOCKET*
	[  Return Value ]: None
*/
DWORD WINAPI ReceiveReplicatorMessageThread(LPVOID param);
