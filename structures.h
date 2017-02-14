#define debug 1

#define SEMAPHORE_RAISE 1
#define SEMAPHORE_DROP -1


#define ROOMS_STRUCTURE_KEY 500
#define ROOMS_SEMAPHORE_KEY 600

#define PLAYERS_STRUCTURE_KEY 501
#define PLAYERS_SEMAPHORE_KEY 601

#define INITIAL_MESSAGE_KEY 1234
#define DEFAULT_RIGHTS 0777
#define SERVER_INTERNAL_QUEUE_KEY 123
#define MAX_PLAYER_NUMBER 100
#define MAX_PID_SIZE 4

#define USER_NAME_LENGTH 50
#define PLAYER_IN_GAME 3
#define PLAYER_AWAITING_FOR_PARTNER 2
#define PLAYER_AWAITING_FOR_ROOM 1
#define PLAYER_DISCONNECTED 0

// Chat consts
#define CHAT_MESSAGE_KEY 503
#define MESSAGE_CONTENT_SIZE 512
#define CHAT_CLIENT_TO_SERVER 3
#define CHAT_SERVER_TO_CLIENT 4

// Lobby consts
#define LOBBY_SIZE 10
#define LOBBY_STRUCTURE_KEY 665
#define LOBBY_SEMAPHORE_KEY 665
#define ROOM_EMPTY 0
#define ROOM_PLAYER_AWAITING 1
#define ROOM_IN_GAME 2
#define MAX_ROOMS_NUMBER MAX_PLAYER_NUMBER/2

// Game consts
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

// Colors <3
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef struct GameMatrix {
    int sem;
    int memKey;
    char *matrix;
} GameMatrix;

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

typedef struct Lobby {
    int sem;
    int memKey;
    Room * rooms;
} Lobby;

typedef struct PlayersMemory {
    int sem;
    int memKey;
    Player * players;
} PlayersMemory;

typedef struct PrivateMessage {
    long type;
    char command[MESSAGE_CONTENT_SIZE];
} PrivateMessage;

typedef struct ChatMessage {
    long type;
    char source[USER_NAME_LENGTH];
    char content[MESSAGE_CONTENT_SIZE];
} ChatMessage;

typedef struct InitialMessage {
    long type;
    int pid;
    char username[USER_NAME_LENGTH];
} InitialMessage;

//used internally on server side
typedef struct ClientInfo
{
	int PID;
	int lobbyIndex;
	char nickname[USER_NAME_LENGTH];
} ClientInfo;

//functions
void resetInitialMessageStructure(InitialMessage *messageToReset);
void resetPrivateMessageStructure(PrivateMessage *privateMessageToReset);
int sendPrivateMessage(int id, PrivateMessage *message); //sends a private message to the client with a message queue of id = id,returns on: success - 1, failure - 0
int receivePrivateMessage(int id, PrivateMessage *message,int messageType); //receives a private message, returns number of bytes received if success, -1 if failed
int sendInitialMessage(int id, InitialMessage *message);
int receiveInitialMessage(int id, InitialMessage *message, int messageType);
int sendChatMessage(int id, ChatMessage *message);
int receiveChatMessage(int id, ChatMessage *message, int messageType);
int getMessageQueue(int key);   //get - create message queue represented by the key

void resetInitialMessageStructure(InitialMessage *messageToReset)
{
	messageToReset->type = 0;
	memset(messageToReset->username, "0",  USER_NAME_LENGTH);
	messageToReset->pid = -1;
}

void resetPrivateMessageStructure(PrivateMessage *privateMessageToReset)
{
	privateMessageToReset->type = 0;
	memset(privateMessageToReset->command, "0",  MESSAGE_CONTENT_SIZE);
}

int sendPrivateMessage(int id, PrivateMessage *message)
{
	if(msgsnd(id, message, sizeof(*message) - sizeof(message->type), 0) == -1)
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
	printf("message length %d\n",sizeof(*message) - sizeof(message->type));	
	    int recivedMessage =  msgrcv(id,message, sizeof(*message) - sizeof(message->type), messageType, 0);
        if(recivedMessage == -1)
        {
        	if(debug)
            	perror("Error after reciving a message:");
            return -1;
        }

}//receives a private message, returns number of bytes received if success, -1 if failed

int sendInitialMessage(int id, InitialMessage *message)
{
		printf("initial send message type %d\n",message->type);	

	if(msgsnd(id, message, sizeof(*message) - sizeof(message->type), 0) == -1)
	{
		if(debug)
			perror("Failed to send private message to the server!");
		return -1;
	}
	else
	{
		return 1;
	}
} //sends a private message to the client with a message queue of id = id

int receiveInitialMessage(int id, InitialMessage *message, int messageType)
{
	printf("Receiving initial message");
	    int recivedMessage =  msgrcv(id,&message, sizeof(*message) - sizeof(message->type), messageType, 0);
        if(recivedMessage == -1)
        {
        	if(debug)
            	perror("Error after reciving a message:");
            return -1;
        }

}//receives a private message, returns number of bytes received if success, -1 if failed


int sendChatMessage(int id, ChatMessage *message)
{
	if(msgsnd(id, message, sizeof(*message) - sizeof(message->type), 0) == -1)
	{
		if(debug)
			perror("Failed to send private message to the server!");
		return -1;
	}
	else
	{
		return 1;
	}
} //sends a private message to the client with a message queue of id = id

int receiveChatMessage(int id, ChatMessage *message, int messageType)
{
	printf("message length %d\n",sizeof(*message) - sizeof(message->type));	
	    int recivedMessage =  msgrcv(id,message, sizeof(*message) - sizeof(message->type), messageType, 0);
        if(recivedMessage == -1)
        {
        	if(debug)
            	perror("Error after reciving a message:");
            return -1;
        }

}//receives a private message, returns number of bytes received if success, -1 if failed


int getMessageQueue(int key)
{
    int initialMessageId = msgget(key, IPC_CREAT | DEFAULT_RIGHTS);
    if(initialMessageId == -1)
    {
        if(errno == EEXIST)
        {
            if(debug)
                perror("Queue already exists");
            initialMessageId = msgget(INITIAL_MESSAGE_KEY, DEFAULT_RIGHTS);
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
	;
}