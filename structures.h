
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
#define PRIVATE_MESSAGE_RIGHTS 0777


//ClientInfo structure 

typedef struct InitialMessage
{
	long mtype;
	char clientsName[CLIENTS_NAME_SIZE + 1];
	int mClientsPID;
} InitialMessage;

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
int sendPrivateMessage(int id, PrivateMessage *message); //sends a private message to the client with a message queue of id = id,returns on: success - 1, failure - 0
int receivePrivateMessage(int id, PrivateMessage *message,int messageType); //receives a private message, returns number of bytes received if success, -1 if failed
int sendInitialMessage(int id, InitialMessage *message);
int receiveInitialMessage(int id, InitialMessage *message, int messageType);
int getMessageQueue(int key);   //get - create message queue represented by the key

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

int sendPrivateMessage(int id, PrivateMessage *message)
{
	if(msgsnd(id, message, PRIVATE_MESSAGE_SIZE, 0) == -1)
	{
		if(debug)
			perror("Failed to send private message to the server!");
		return 0;
	}
	else
	{
		return 1;
	}
} //sends a private message to the client with a message queue of id = id

int receivePrivateMessage(int id, PrivateMessage *message, int messageType)
{
	    int recivedMessage =  msgrcv(id,message, PRIVATE_MESSAGE_SIZE, messageType, 0);
        if(recivedMessage == -1)
        {
        	if(debug)
            	perror("Error after reciving a message:");
            return -1;
        }

}//receives a private message, returns number of bytes received if success, -1 if failed

int sendInitialMessage(int id, InitialMessage *message)
{
	if(msgsnd(id, message, INITIAL_MESSAGE_SIZE, 0) == -1)
	{
		if(debug)
			perror("Failed to send private message to the server!");
		return 0;
	}
	else
	{
		return 1;
	}
} //sends a private message to the client with a message queue of id = id

int receiveInitialMessage(int id, InitialMessage *message, int messageType)
{
	    int recivedMessage =  msgrcv(id,&message, INITIAL_MESSAGE_SIZE, messageType, 0);
        if(recivedMessage == -1)
        {
        	if(debug)
            	perror("Error after reciving a message:");
            return -1;
        }

}//receives a private message, returns number of bytes received if success, -1 if failed

int getMessageQueue(int key)
{
    int initialMessageId = msgget(key, IPC_CREAT | MESSAGE_QUEUE_RIGHTS);
    if(initialMessageId == -1)
    {
        if(errno == EEXIST)
        {
            if(debug)
                perror("Queue already exists");
            initialMessageId = msgget(INITIAL_MESSAGE_KEY, MESSAGE_QUEUE_RIGHTS);
            return initialMessageId;
        }
        else
        {
            perror("error getting message queue in findNewClients serwer : ");
            return -1;
        }
    }
    return initialMessageId;
}   //get - create message queue represented by the key