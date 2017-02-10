#define true 1
#define false 0

#define maxClients 20
#define maxRooms 10

int initialMessageKey = 9899;
int initialMessageSize = 14;
typedef struct
{
	long mtype;
	char mtext[10];
	int mClientsPID;
} InitialMessage;

int privateMessageSize = 10;
typedef struct 
{
	long mtype;
	char mtext[10];
} PrivateMessage;

