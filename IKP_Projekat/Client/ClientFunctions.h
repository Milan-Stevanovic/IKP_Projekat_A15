/*
	Function: InitializeWindowsSockets
	------------------------------------
	[ Functionality ]: Initialize WsaData
	[     Params    ]: None
	[  Return Value ]: success of the action -> bool
*/
bool InitializeWindowsSockets();


/*
	Function: Menu
	------------------------------------
	[ Functionality ]: Prints all available options presented to client to choose
	[     Params    ]: None
	[  Return Value ]: choosen option -> int
*/
int Menu();

/*
	Function: ConnectToServer
	------------------------------------
	[ Functionality ]: Make socket and connect to server
	[     Params    ]: port -> int
	[  Return Value ]: connected socket -> SOCKET
*/
SOCKET ConnectToServer(int port);

/*
	Function: SetSocketToNonBlockingMode
	------------------------------------
	[ Functionality ]: Set socket to non blocking mode
	[     Params    ]: socket -> SOCKET
	[  Return Value ]: None
*/
void SetSocketToNonBlockingMode(SOCKET socket);

/*
	Function: IsClientReigistered
	------------------------------------
	[ Functionality ]: Checks if client is registered (id != -1)
	[     Params    ]: clientId -> int
	[  Return Value ]: bool
*/
bool IsClientRegistered(int clientId);


/*
	Function: ReceiveServerMessageThread
	------------------------------------
	[ Functionality ]: Thread that is used to receive messages from server
	[     Params    ]: socket -> SOCKET*
	[  Return Value ]: None
*/
DWORD WINAPI ReceiveServerMessageThread(LPVOID param);