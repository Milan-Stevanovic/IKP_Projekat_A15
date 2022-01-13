/*
	Function: ConnectClientsThread
	------------------------------------
	[ Functionality ]: Threades function for accepting clientcopy connections
	[     Params    ]: clientListenSocket -> SOCKET*
	[  Return Value ]: None
*/
DWORD WINAPI ConnectClientsThread(LPVOID param);

/*
	Function: ClientMessageThread
	------------------------------------
	[ Functionality ]: Reveives all messages from client copies
	[     Params    ]: None
	[  Return Value ]: None
*/
DWORD WINAPI ClientMessageThread(LPVOID param);

/*
	Function: ServerMessageThread
	------------------------------------
	[ Functionality ]: Reveives all messages from server
	[     Params    ]: serverSocket -> SOCKET*
	[  Return Value ]: None
*/
DWORD WINAPI ServerMessageThread(LPVOID param);

/*
	Function: PassServerMessageThread
	------------------------------------
	[ Functionality ]: Get message from ring buffer () and pass it to CopyClient
	[     Params    ]: None
	[  Return Value ]: None
*/
DWORD WINAPI PassServerMessageThread(LPVOID param);

/*
	Function: PassMessageFromClientToServer
	------------------------------------
	[ Functionality ]: Get message from ring buffer and pass it to Server
	[     Params    ]: serverSocket -> SOCKET*
	[  Return Value ]: None
*/
DWORD WINAPI PassMessageFromClientToServer(LPVOID param);

/*
	Function: InitializeWindowsSockets
	------------------------------------
	[ Functionality ]: Set up WsaData params
	[     Params    ]: None
	[  Return Value ]: bool
*/
bool InitializeWindowsSockets();

/*
	Function: LaunchClientCopy
	------------------------------------
	[ Functionality ]: In code start up of ClientCopy.exe
	[     Params    ]: id -> int
	[  Return Value ]: None
*/
void LaunchClientCopy(int clientId);

/*
	Function: SetUpListenSockets
	------------------------------------
	[ Functionality ]: Sets up listen sockets for specific port
	[     Params    ]: port -> const char*
	[  Return Value ]: new listenSocket[Client/Server ones] -> SOCKET
*/
SOCKET SetUpListenSockets(const char* port);

/*
	Function: SetSocketToNonBlockingMode
	------------------------------------
	[ Functionality ]: Set socket to non blocking mode
	[     Params    ]: socket -> SOCKET
	[  Return Value ]: None
*/
void SetSocketToNonBlockingMode(SOCKET socket);

