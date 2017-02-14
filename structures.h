
#include <stdio.h>
#define debug 1
#define true 1
#define false 0

//sending messages
#define SERVER_2_CLIENT_TYPE 2
#define CLIENT_2_SERVER_TYPE 1

//lobby constants
#define MAX_CLIENTS_NUMBER 20
#define MAX_ROOMS_NUMBER 10

//initial message constants
#define INITIAL_MESSAGE_KEY  1234
#define MESSAGE_QUEUE_RIGHTS 0777
#define CLIENTS_NAME_SIZE 20

#define INITIAL_MESSAGE_TEXT_SIZE 21
#define INITIAL_MESSAGE_SIZE INITIAL_MESSAGE_TEXT_SIZE+7


//private message constants
#define PRIVATE_MESSAGE_KEY 0
#define PRIVATE_MESSAGE_SIZE 512
#define PRIVATE_MESSAGE_RIGHTS 0777

// Game consts - od Witka
#define GAME_CLIENT_TO_SERVER 1
#define GAME_SERVER_TO_CLIENT 2
#define GAME_WANT_TO_CONTINUE 5
#define GAME_MATRIX_SIZE 4
#define GAME_ROOM_KEY_ADDER 50
#define GAME_PLAYER_0_SIGN 'x'
#define GAME_PLAYER_1_SIGN 'o'
#define GAME_FINISHED 3
#define GAME_YOUR_TOUR 2
#define GAME_MOVE_ACCEPTED 1
#define GAME_MOVE_REJECTED 0

//Players structure 
#define PLAYERS_STRUCTURE_KEY 500
#define PLAYERS_SEMAPHORE_KEY 600


//Rooms structure 
#define ROOMS_STRUCTURE_KEY 501
#define ROOMS_SEMAPHORE_KEY 601

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
	char mtext[PRIVATE_MESSAGE_SIZE];
} PrivateMessage;



typedef struct ClientInfo
{
	int PID;
	int lobbyIndex;
	char nickname[CLIENTS_NAME_SIZE + 1];
} ClientInfo;


typedef struct Player {
    int pid;
    int queueId;
    char name[USER_NAME_LENGTH];
    int state;
} Player;

typedef struct Room {
    int state;
    Player players[2];
} Room;

typedef struct GameArray
{
	
}GameArray;

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

void showRoomsContent()
{
	
}