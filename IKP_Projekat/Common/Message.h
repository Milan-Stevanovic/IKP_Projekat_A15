#define DEFAULT_BUFLEN 512

enum flag
{ 
	REGISTER = 0, 
	SEND_DATA = 1, 
	RECIEVE_DATA = 2, 
	REQUEST_INTEGRITY_UPDATE = 3, 
	RELAUNCH_COPY = 4 
};

struct Message
{
	int id;
	int flag;
	char data[256];
};