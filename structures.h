
#include <stdio.h>
#define true 1
#define false 0

#define maxClients 20
#define maxRooms 10
#define MESSAGE_QUEUE_RIGHTS 0777

int INITIAL_MESSAGE_KEY = 9899;

int initialMessageSize = 14;
typedef struct InitialMessage
{
	long mtype;
	char mtext[10];
	int mClientsPID;
} InitialMessage;

int privateMessageSize = 10;
typedef struct PrivateMessage
{
	long mtype;
	char mtext[10];
} PrivateMessage;

