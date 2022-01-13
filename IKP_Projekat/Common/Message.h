enum flag
{ 
	REGISTER = 0, 
	SEND_DATA = 1, 
	REC_DATA = 2, 
	INTEGRITY_UPDATE = 3, 
	RELAUNCH_COPY = 4, 
	REQ_REC_DATA = 5,
	REQ_INTEGRITY_UPDATE = 6
};

struct Message
{
	int id;
	int flag;
	char data[256];
};