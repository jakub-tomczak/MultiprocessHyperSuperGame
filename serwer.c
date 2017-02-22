#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>	//errno
#include <stdlib.h> //exit
#include <stdbool.h> //bool expressions
#include "structures.h"


//functions
void findNewClients(Lobby *lobby, Players *players, GameMatrix *gameMatrix);
void addNewClient(InitialMessage *newClient, Players *players, int *clientsArrayIndex); 
int findClientByPID(int clientPID, Players *clientsArray); //return index of a client with PID in array of clientInfo structures

    ///SHARED MEMORY
int initializeRooms(Room *rooms);      //returns id to shm
void prepareLobbyDataToSend(Lobby *lobby, char *message);
void spreadChatMessage(int senderPID, Players *players, ChatMessage * message);
int addClientToRoom(Lobby *lobby, Players *players, int clientIndex, int roomIndex);
Lobby initializeLobby();
Players initializePlayers();
GameMatrix initializeGameMatrix();
int canJoinRoom(Lobby *lobby, int roomIndex);

    ///SEMAPHORES
int roomLock();             //group of sem to rooms
int playersLimitLock();     //returns id to lock sem of max players
int roomsShm();
char * roomsAdr(int roomShmID);
void manageGame(Lobby *lobby, Players * players,int roomJoining);

//globals
int currentNumberOfClients_GLOBAL = 0;
int serverPID ;
struct sembuf semaphore;

int main(int argc, char * argv [])
{
    Lobby lobby;
    Players players;
    GameMatrix gameMatrix;

    lobby = initializeLobby();
    players = initializePlayers();
    gameMatrix = initializeGameMatrix();

    findNewClients(&lobby, &players, &gameMatrix);
    return 0;

}


void findNewClients(Lobby *lobby, Players *players, GameMatrix *gameMatrix)
{
    InitialMessage  message2Rcv;
    
    int currentNumberOfClients = 0;
    bool initialMessageDestroyed = false;

    int initialMessageId = getMessageQueue(INITIAL_MESSAGE_KEY);
                int check = msgctl(initialMessageId, IPC_RMID, NULL);
    
        if(check == -1)
        {
           perror("Msgctl error in findNewClients serwer:");
        }
        else
        {
            if(debug)
                 printf("Successfully deleted a queue\n");
        }
             initialMessageId = getMessageQueue(INITIAL_MESSAGE_KEY);

    if(initialMessageId == -1)
    {
     
            perror("error getting message queue in findNewClients serwer : ");
            exit(0);
    
    }

    while(true)
    {
        if(currentNumberOfClients == MAX_PLAYER_NUMBER)
        {
            printf("Max number of clients exceded!\n");
            //tutaj walnąć semafor który zamkniemy, usuniecie klienta spowoduje podniesienie semafora  v  ;
        }
       // message2Rcv.pid = -1;
       // int message = msgrcv(initialMessageId, &message2Rcv);
        int recivedMessage =  receiveInitialMessage(initialMessageId, &message2Rcv, GAME_CLIENT_TO_SERVER);
        if(recivedMessage == -1)
        {

            perror("Error after reciving a message:");

            break;
        }
        else
        {
                            addNewClient(&message2Rcv, players, &currentNumberOfClients);

            //new client hass been found!
            int newClientFork = fork();
            if(newClientFork == 0)
            {
                //begin new client found section


                ClientInfo client;
                client.PID = message2Rcv.pid;
                //childs process - keep private communication with a client in that process
                int privateMessageID = getMessageQueue(message2Rcv.pid);
                if(fork() == 0)
                {
                    //chat listener
                    //chat server
                    int chatQueue = getMessageQueue(message2Rcv.pid);
                    if(chatQueue == -1)
                    {   
                        printf("Failed to start chat\n");
                        exit(0);
                    }

                    while(true)
                    {
                        ChatMessage receivedChatMessage;
                        int receivedMessage = receiveChatMessage(privateMessageID, &receivedChatMessage, CHAT_CLIENT_TO_SERVER);
                        if(receivedMessage ==-1)
                        {
                              printf("Failed to receive chat message\n");
                              break;
                        }
                       // printf("message from chat %s, sender: %s\n", receivedChatMessage.content, receivedChatMessage.username);
                        
                        receivedChatMessage.type = CHAT_SERVER_TO_CLIENT;

                        for(int i=0;i<MAX_PLAYER_NUMBER;i++)
                        {

                            printf("%d: %d\n", i, players->clients[i].PID);
                            int pid = players->clients[i].PID;
                            if(pid == message2Rcv.pid) continue;
                            if(pid > 0)
                            {
                                int msgID = getMessageQueue(pid);
                                if(msgID == -1) continue;

                                int msgSND = sendChatMessage(msgID, &receivedChatMessage);
                                if(msgSND == -1) continue;
                            }
                        }
    



                        //spreadChatMessage(message2Rcv.pid, players, &receivedChatMessage);

                    }


                }else{
                  //   printf("Client's private message id: %d, clitns pid %d\n", privateMessageID, message2Rcv.pid);
                    if(privateMessageID == -1)
                    {
                         printf("Couldn't launch message queue for client with PID: %d, exiting...\n", message2Rcv.pid);
                         break;
                    }
                    else
                    {
                          printf("Got private message queue %d\n", privateMessageID);
                    }

                    resetInitialMessageStructure(&message2Rcv); //clears message2Rcv

                     //sends message with id of shared memory - with rooms
                    char pidChar[6]; 
                    sprintf(pidChar,"%d", serverPID);
                    PrivateMessage newPrivateMessage;
                    newPrivateMessage.type = GAME_SERVER_TO_CLIENT;

                    strcpy(newPrivateMessage.content, pidChar);
                                 
                    if(sendPrivateMessage(privateMessageID, &newPrivateMessage) == -1)
                    {
                      printf("Error when sending private message to the client!\n");
                    }
                    else
                    {
                        if(debug)
                            printf("Send private message Successfully \n");
                    }
                    

                    resetPrivateMessageStructure(&newPrivateMessage);
                    
                    char message[MESSAGE_CONTENT_SIZE];
                    prepareLobbyDataToSend(lobby, message);
                    strcpy(newPrivateMessage.content, message);
                    newPrivateMessage.type = GAME_SERVER_TO_CLIENT;

                   
                    if(sendPrivateMessage(privateMessageID, &newPrivateMessage) == -1)
                    {
                        printf("Error when sending private message to the client!\n");
                        break;
                    }
                    else
                    {
                        if(debug)
                            printf("Send lobby message Successfully \n");
                    }
                    printf("message id %d\n", privateMessageID);
                    resetPrivateMessageStructure(&newPrivateMessage);
                    newPrivateMessage.type = GAME_CLIENT_TO_SERVER;
                    
                    int roomResponse = -1;
                    int roomJoining = -1;
                    do
                    {

                        roomResponse = -1;
                        int temp;
                        if(receivePrivateMessage(privateMessageID, &newPrivateMessage ,GAME_CLIENT_TO_SERVER) != -1)
                        {
                            temp = atoi(newPrivateMessage.content);
                            roomResponse = canJoinRoom(lobby, temp);
                        }
                        else
                        {
                            perror("receiving room info failure");
                        }

                        newPrivateMessage.type = GAME_SERVER_TO_CLIENT;
                        switch(roomResponse)
                        {
                            case 1: //choosen room is correct
                                
                                
                                if(addClientToRoom(lobby, players, client.PID, temp) <0)
                                {
                                    roomResponse = -1;
                                    strcpy(newPrivateMessage.content, "x");
                                }
                                else
                                {
                                    roomJoining = roomResponse;                                    
                                }
                                break;
                            case 0: //room is in game
                                strcpy(newPrivateMessage.content,"x");
                                break;

                            case -1: //room number is invaild
                                strcpy(newPrivateMessage.content,"x");
                                break;
                        }
                        if(sendPrivateMessage(privateMessageID, &newPrivateMessage) == -1)
                        {
                            printf("Failed to send message\n" );
                        }
                      
                        
                    }while(roomResponse == 0 || roomResponse == -1);
                    
                    if(lobby->rooms[roomJoining].state == ROOM_IN_GAME)
                    {
                        if(fork() == 0)
                        {
                            printf("Starting game in room %d\n", roomJoining);
                            manageGame(lobby, players, roomJoining);
                        }
                    }
                    
                    
                }




                //end new client found section

            }
            else
            {
                //go and look for next client
            }

               

        }

    }


    //delete message queue after the error
    
    if(initialMessageDestroyed == false)
    {
        int check = msgctl(initialMessageId, IPC_RMID, NULL);
    
        if(check == -1)
        {
           perror("Msgctl error in findNewClients serwer:");
        }
        else
        {
            if(debug)
                 printf("Successfully deleted a queue\n");
        }
    }
    initialMessageDestroyed = true;


}

void addNewClient(InitialMessage *newClient, Players *players, int *clientsArrayIndex)
{
   // enterPlayersOperation(players);
    ClientInfo newClientInfo;
    newClientInfo.PID = newClient->pid;
    newClientInfo.roomIndex = -1;
    strcpy(newClientInfo.nickname, newClient->username);
    players->clients[(*clientsArrayIndex)++] = newClientInfo;
    printf("\nTotal number of clients = %d, pid %d", (*clientsArrayIndex), getpid());
    currentNumberOfClients_GLOBAL = *clientsArrayIndex;
    //leavePlayersOperation(players);
   // displayPlayers(players);
}

int addClientToRoom(Lobby *lobby, Players *players, int clientIndex, int roomIndex)
{
    int playerIndex = findClientByPID(clientIndex, players);
    if(playerIndex  < 0) return -1;
   // if(playerIndex == -1) return -1;
    //trzeba wyswietlic klientow
    printf("Waiting for an access\n");
   // displayPlayers(players);
    //semaphores
   // enterPlayersOperation(players);
    enterLobbyMemory(lobby);
    printf("Entered critical section\n");
    //sleep(5);
    int returnValue = -1;
    if(lobby->rooms[roomIndex].state == ROOM_EMPTY)
    {
        lobby->rooms[roomIndex].players[0] = players->clients[playerIndex];
        lobby->rooms[roomIndex].state = ROOM_PLAYER_AWAITING;
        returnValue = 1;
    }else if(lobby->rooms[roomIndex].state == ROOM_PLAYER_AWAITING)
    {
        lobby->rooms[roomIndex].players[1] = players->clients[playerIndex];
        lobby->rooms[roomIndex].state = ROOM_IN_GAME;
        returnValue = 2;
    }
    if(returnValue != -1)
    {
        players->clients[playerIndex].roomIndex = roomIndex;
    printf("Added player %s to the room %d\n", players->clients[playerIndex].nickname, players->clients[playerIndex].roomIndex);
    }
    leaveLobbyMemory(lobby);
    //leavePlayersOperation(players);
    printf("Exited\n");
    return returnValue;
}

int findClientByPID(int clientPID, Players *clientsArray)
{
   // enterPlayersOperation(clientsArray);
    ClientInfo *client = clientsArray->clients;
    for(int indx = 0 ; indx < MAX_PLAYER_NUMBER; ++indx, client++)
    {
       if(client->PID == clientPID)
       {
            printf("Found at %d\n", indx);
            //leavePlayersOperation(clientsArray);
            return indx;
       }
    }
    printf("Not found\n");

    //leavePlayersOperation(clientsArray);
    return -1;
} //return index of a client with PID in array of clientInfo structures

int canJoinRoom(Lobby *lobby, int roomIndex)
{
    if(roomIndex == -1) return -1;
    switch(lobby->rooms[roomIndex].state)
    {
        case ROOM_EMPTY:
            return 1;
        case ROOM_PLAYER_AWAITING:
            return 1;
        case ROOM_IN_GAME:
            return 0;
        default:
            return -1;
    }
    return -1;  
}



int initializeRooms(Room *rooms)
{
    Room room;
    int roomShmID = shmget(ROOM_MEMORY_KEY, sizeof(room)*MAX_ROOMS_NUMBER, IPC_CREAT | DEFAULT_RIGHTS);

    rooms = shmat(roomShmID, NULL,0);
    for(int i=0;i<MAX_ROOMS_NUMBER;i++)
    {
        rooms[i] = getEmptyRoom();
    }
    return roomShmID;
}     //returns id to shm

void displayPlayers(Players *playersArray)
{
    printf("%5s\n", "-");
    ClientInfo *player = playersArray->clients;
    for(int i=0;i<MAX_PLAYER_NUMBER;i++, player++)
    {
        if(player->PID == -1) 
            printf("%d) <none>\n", i);
        else
            printf("%d) Player %s, pid: %d\n", i, player->nickname, player->PID);
    }
    printf("%5s\n", "-");
}




///FUNCTIONS FOR SEMAPHORES
int roomLock()
{
    int semaphoreID = semget(ROOMS_SEMAPHORE_KEY, 1, IPC_CREAT | DEFAULT_RIGHTS);
    if(semaphoreID == -1)
    {
        perror("Failed to initialize semaphore for rooms: ");
        return -1;
    }
    return semaphoreID;
}            //group of sem to rooms
int playersLimitLock()
{
    int semaphoreID = semget(PLAYERS_SEMAPHORE_KEY, MAX_PLAYER_NUMBER, IPC_CREAT | 0777);
    if(semaphoreID == -1)
    {
        perror("Failed to initialize semaphore for rooms: ");
        return -1;
    }
    return semaphoreID;
}     //returns id to lock sem of max players



void prepareLobbyDataToSend(Lobby *lobby, char *message)
{
    enterLobbyMemory(lobby);
    strcpy(message, "");
    char number[2];
    for(int i=0;i<MAX_ROOMS_NUMBER;i++)
    {
        //add number of room at the beginning
        sprintf(number, "%d", i);
        strcat(message, number);
        strcat(message, ":");
       //    printf("%s, i=%d\n", *message, i);
        switch(lobby->rooms[i].state)
        {
            case ROOM_EMPTY:
                strcat(message, "<empty>");
                break;
            case ROOM_PLAYER_AWAITING:
                strcat(message,lobby->rooms[i].players[0].nickname);
                break;
            case ROOM_IN_GAME:
                strcat(message, "<game>");
                break;
            default:
                break;

        }
        if(i < MAX_ROOMS_NUMBER - 1)
            strcat(message, "\n");
    } 

    leaveLobbyMemory(lobby);
}

void enterLobbyMemory(Lobby *lobby)
{
    semaphoreOperation(lobby->semID, SEM_P);
}

void leaveLobbyMemory(Lobby *lobby)
{
    semaphoreOperation(lobby->semID, SEM_V);
}

void enterPlayersOperation(Players *players)
{   
    semaphoreOperation(players->semID, SEM_P);
}
void leavePlayersOperation(Players *players)
{   
    semaphoreOperation(players->semID, SEM_V);
}

void semaphoreOperation(int semId, int operation) {
    semaphore.sem_op = operation;
    semop(semId, &semaphore, 1);
}

void manageGame(Lobby *lobby, Players * players,int roomJoining)
{
    int gameKey = roomJoining + 50;

    printf("Players in a new game: %s, %s, in the room no %d, game shm, sem key = %d\n", 
        lobby->rooms[roomJoining].players[0].nickname,
         lobby->rooms[roomJoining].players[1].nickname,
         roomJoining,
         gameKey); 
    bool gameNotFinished = true;
    int firstPID = lobby->rooms[roomJoining].players[0].PID;
    int secondPID = lobby->rooms[roomJoining].players[1].PID;

    GameMatrix gameMatrix;
    //semaphore
    gameMatrix.semID = semget(gameKey, 1, IPC_CREAT | DEFAULT_RIGHTS);
    semctl(gameMatrix.semID, 0, SETVAL, 1);

    //shared memory
    int shmID = shmget(gameKey, GAME_MATRIX_CELLS, IPC_CREAT | DEFAULT_RIGHTS );
    gameMatrix.memID = shmID;
    gameMatrix.board = shmat(gameMatrix.memID, 0, 0);
    printf("shm id = %d\n", gameMatrix.memID);

    semaphoreOperation(gameMatrix.semID, SEM_P);

    char tempMatrix[GAME_MATRIX_CELLS];
    memset(tempMatrix, (char)32, GAME_MATRIX_CELLS);
    strcpy(gameMatrix.board, tempMatrix);
   
    semaphoreOperation(gameMatrix.semID, SEM_V);

    //private queues ids
    int firstQueueID = msgget(firstPID, DEFAULT_RIGHTS);
    int secondQueueID = msgget(secondPID, DEFAULT_RIGHTS);
    printf("Sending first messages\n");

    PrivateMessage gameMessage;
    gameMessage.type = GAME_SERVER_TO_CLIENT;
    strcpy(gameMessage.content, "2");
    sendPrivateMessage(firstQueueID, &gameMessage);

    int turnPlayer = firstQueueID;
    perror("cos nie wyszlo podczas wysylania");

    printf("Starting new game %s, waiting for id %d\n", gameMessage.content, turnPlayer);
    while(gameNotFinished)
    {   

        resetPrivateMessageStructure(&gameMessage);
        gameMessage.type = GAME_CLIENT_TO_SERVER;

       // int a = msgrcv(turnPlayer, &gameMessage, sizeof(gameMessage)-sizeof(gameMessage.type), GAME_CLIENT_TO_SERVER, 0);
        if(receivePrivateMessage(turnPlayer, &gameMessage, GAME_CLIENT_TO_SERVER) == -1)
        {
                        printf("Connection broken!\n");
                        msgctl(turnPlayer, IPC_RMID, NULL);
                        semctl(gameMatrix.semID, 0, IPC_RMID, NULL);
                        shmctl(gameMatrix.memID, IPC_RMID, NULL);
                        break;
        }
      
//        perror("cos");
  //      printf("a %d, type: %d, exact message :>%s<\n", a, gameMessage.type, gameMessage.content);
    //    printf("Message from player after choosing column numer: %s\n", gameMessage.content);
        gameMessage.type = GAME_SERVER_TO_CLIENT;
        printf("message %s\n", gameMessage.content);
        if(isdigit(gameMessage.content[0]))
        {
            int choosenColumn = atoi(gameMessage.content);
            printf("Ruch jest ok!");

            strcpy(gameMessage.content, "1");   //accepted
            sendPrivateMessage(turnPlayer, &gameMessage);
            //change player
            if(turnPlayer == firstQueueID)
                turnPlayer = secondQueueID;
            else
                turnPlayer = firstQueueID;
            strcpy(gameMessage.content, "2");   //tell another player that is its turn
            sendPrivateMessage(turnPlayer, &gameMessage);   
        }
        else
        {
            strcpy(gameMessage.content, "0");
            sendPrivateMessage(turnPlayer, &gameMessage);
            //ponow ruch - nie zmieniaj gracza
        }
    }

}

void spreadChatMessage(int senderPID, Players *players, ChatMessage *message)
{

    message->type = CHAT_SERVER_TO_CLIENT;
    strcpy(message->content, message);

    for(int i=0;i<MAX_PLAYER_NUMBER;i++)
    {

        printf("%d: %d\n", i, players->clients[i].PID);
        int pid = players->clients[i].PID;
        if(pid == senderPID) continue;
        if(pid > 0)
        {
            int msgID = getMessageQueue(pid);
            if(msgID == -1) continue;

            int msgSND = sendChatMessage(msgID, message);
            if(msgSND == -1) continue;
        }
    }
}


Lobby initializeLobby()
{
    Lobby lobby;
    Room room;
    int roomsMemoryID = shmget(ROOM_MEMORY_KEY, sizeof(Room) * MAX_ROOMS_NUMBER, IPC_CREAT | DEFAULT_RIGHTS);
    int lobbySemaphore = semget(LOBBY_SEMAPHORE_KEY, 1, IPC_CREAT | DEFAULT_RIGHTS);

    lobby.shmID = roomsMemoryID;
    lobby.semID = lobbySemaphore;

    lobby.rooms = shmat(roomsMemoryID, NULL, 0);
    for(int i=0;i<MAX_ROOMS_NUMBER;i++)
    {
        lobby.rooms[i] = getEmptyRoom();
    }
    semctl(lobby.semID, 0, SETVAL, 1);
    return lobby;
}
Players initializePlayers()
{
    Players players;
    ClientInfo *clients, client;
    int clientsMemID = shmget(PLAYERS_STRUCTURE_KEY, sizeof(client) * MAX_PLAYER_NUMBER, IPC_CREAT | DEFAULT_RIGHTS);
    int clientsSemID = semget(PLAYERS_SEMAPHORE_KEY, 1, IPC_CREAT | DEFAULT_RIGHTS);

    players.memID = clientsMemID;
    players.semID = clientsSemID;

    players.clients = shmat(players.memID, NULL,0);

    for(int i=0;i<MAX_PLAYER_NUMBER;i++)
    {
        players.clients[i] = getEmptyClientInfo();
    }

    semctl(players.semID, 0, SETVAL, 1);

    return players;
}

GameMatrix initializeGameMatrix()
{
    GameMatrix gameMatrix;
    gameMatrix.memID = shmget(GAME_MATRIX_STRUCTURE_KEY, sizeof(char) * GAME_MATRIX_CELLS, IPC_CREAT | DEFAULT_RIGHTS);
    if(gameMatrix.memID ==  -1)
        printf("Failed to allocate shared memory for game matrix\n");
    if(gameMatrix.semID == -1)
        printf("Failed to allocate semaphore memory for game matrix\n");
    gameMatrix.board = shmat(gameMatrix.memID, NULL, 0);

    semctl(gameMatrix.semID, 0, SETVAL, 2);
}