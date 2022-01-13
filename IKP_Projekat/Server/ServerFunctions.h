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
	[ Functionality ]: Makes new socket for communication with replicator and connects to replicator.
	[     Params    ]: ( Replicators ) port -> int
	[  Return Value ]: replicatorSocket -> SOCKET
*/
SOCKET ReplicatorConnection(int port);

/*
	Function: SetSocketToNonBlockingMode
	------------------------------------
	[ Functionality ]: Set socket to non blocking mode
	[     Params    ]: socket -> SOCKET
	[  Return Value ]: None
*/
void SetSocketToNonBlockingMode(SOCKET socket);

/*
	Function: ClientConnectionThread
	------------------------------------
	[ Functionality ]: Thread listen for new clients connection, accepts them and adds them to ClientList.
	[     Params    ]: listenSocket -> SOCKET *
	[  Return Value ]: None
*/
DWORD WINAPI ClientConnectionThread(LPVOID param);

/*
	Function: ReceiveClientMessageThread
	------------------------------------
	[ Functionality ]: Receives messages from clients, process the message and puts message in ring buffer to pass it to replicator.
	[     Params    ]: None
	[  Return Value ]: None
*/
DWORD WINAPI ReceiveClientMessageThread(LPVOID param);

/*
	Function: PassMessageFromClientToReplicatorThread
	------------------------------------
	[ Functionality ]: Reads message from ring buffer and passes it to Replicator
	[     Params    ]: replicatorSocket -> SOCKET*
	[  Return Value ]: None
*/
DWORD WINAPI PassMessageFromClientToReplicatorThread(LPVOID param);

/*
	Function: ReceiveReplicatorMessageThread
	------------------------------------
	[ Functionality ]: Receives all messages from replicator.
	[     Params    ]: replicatorSocket -> SOCKET *
	[  Return Value ]: None
*/
DWORD WINAPI ReceiveReplicatorMessageThread(LPVOID param);

/*
	Function: PassMessageFromReplicatorToClientThread
	------------------------------------
	[ Functionality ]: If ring buffer isn't empty, gets first message in ring and passes it to coresponding client.
	[     Params    ]: None
	[  Return Value ]: None
*/
DWORD WINAPI PassMessageFromReplicatorToClientThread(LPVOID param);

/*
	Function: SetUpListenSockets
	------------------------------------
	[ Functionality ]: Makes new socket and set it to listen on port that passed by function call.
	[     Params    ]: port -> const char*
	[  Return Value ]: SOCKET -> listenSocket
*/
SOCKET SetUpListenSockets(const char* port);