
#include <stdio.h>
#define debug 1
#define true 1
#define false 0

//lobby constans
#define MAX_CLIENTS_NUMBER 20
#define MAX_LOBBY_NUMBER 10

//initial message constans
#define INITIAL_MESSAGE_KEY  1234
#define MESSAGE_QUEUE_RIGHTS 0777
#define CLIENTS_NAME_SIZE 20

#define INITIAL_MESSAGE_TEXT_SIZE 21
#define INITIAL_MESSAGE_SIZE INITIAL_MESSAGE_TEXT_SIZE+7


//private message constants
#define PRIVATE_MESSAGE_KEY 0
#define PRIVATE_MESSAGE_SIZE 10 + 1


//ClientInfo structure 

typedef struct InitialMessage
{
	long mtype;
	char clientsName[CLIENTS_NAME_SIZE + 1];
	int mClientsPID;
} InitialMessage;

int privateMessageSize = 10;
typedef struct PrivateMessage
{
	long mtype;
	char mtext[PRIVATE_MESSAGE_SIZE + 1];
} PrivateMessage;



typedef struct ClientInfo
{
	int PID;
	int lobbyIndex;
	char nickname[CLIENTS_NAME_SIZE + 1];
} ClientInfo;


void resetInitialMessageStructure(InitialMessage *messageToReset);
void resetPrivateMessageStructure(PrivateMessage *privateMessageToReset);

void resetInitialMessageStructure(InitialMessage *messageToReset)
{
	messageToReset->mtype = 0;
	memset(messageToReset->clientsName, "0",  CLIENTS_NAME_SIZE+1);
	messageToReset->mClientsPID = -1;
}

void resetPrivateMessageStructure(PrivateMessage *privateMessageToReset)
{
	privateMessageToReset->mtype = 0;
	memset(privateMessageToReset->mtext, "0",  CLIENTS_NAME_SIZE+1);
}
