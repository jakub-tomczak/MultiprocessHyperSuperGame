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
void findNewClients(Lobby *lobby, Players *players);
void addNewClient(InitialMessage *newClient, Players *players, int *clientsArrayIndex); 
int findClientByPID(int clientPID, ClientInfo *clientsArray); //return index of a client with PID in array of clientInfo structures

    ///SHARED MEMORY
int initializeRooms(Room *rooms);      //returns id to shm
void prepareLobbyDataToSend(Lobby *lobby, char *message);
void spreadChatMessage(int senderPID, int playersShmID);

Lobby initializeLobby();
Players initializePlayers();

    ///SEMAPHORES
int roomLock();             //group of sem to rooms
int playersLimitLock();     //returns id to lock sem of max players
int roomsShm();
char * roomsAdr(int roomShmID);


//globals
int currentNumberOfClients_GLOBAL = 0;
int serverPID ;
struct sembuf semaphore;

int main(int argc, char * argv [])
{
    Lobby lobby;
    Players players;

    lobby = initializeLobby();
    players = initializePlayers();

    findNewClients(&lobby, &players);
    return 0;

}


void findNewClients(Lobby *lobby, Players *players)
{
    InitialMessage  message2Rcv;
    
    int currentNumberOfClients = 0;
    bool initialMessageDestroyed = false;
    int initialMessageId = getMessageQueue(INITIAL_MESSAGE_KEY);
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
            //new client hass been found!

            addNewClient(&message2Rcv, players, &currentNumberOfClients);



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
                        printf("message from chat %s\n", receivedChatMessage.content);
                        //spreadChatMessage(message2Rcv.pid, playersShmID);

                    }


                }else{
                     printf("Client's private message id: %d, clitns pid %d\n", privateMessageID, message2Rcv.pid);
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
                    }
                    else
                    {
                        if(debug)
                            printf("Send lobby message Successfully \n");
                    }
                    break;  //to delete queue

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
    ClientInfo newClientInfo;
    newClientInfo.PID = newClient->pid;
    newClientInfo.roomIndex = -1;
    strcpy(newClientInfo.nickname, newClient->username);
    players->clients[(*clientsArrayIndex)++] = newClientInfo;
    currentNumberOfClients_GLOBAL = *clientsArrayIndex;
}

int findClientByPID(int clientPID, ClientInfo *clientsArray)
{
    for(int indx = 0 ; indx < currentNumberOfClients_GLOBAL; ++indx)
    {
       if(clientsArray[indx].PID == clientPID)
       {
            return indx;
       }
    }
    return -1;
} //return index of a client with PID in array of clientInfo structures





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
    //enterLobbyMemory(lobby);
    strcpy(message, "");

    for(int i=0;i<MAX_ROOMS_NUMBER;i++)
    {
        //add number of room at the beginning
        char number[2];
        sprintf(number, "%s", i);
        strcat(message, number);
        strcat(message, ":");
        printf("%s, i=%d\n", message, i);
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
            strcat(message, ";");
    } 
    
   
    printf("Done");

    //leaveLobbyMemory(lobby);
}

void enterLobbyMemory(Lobby *lobby)
{
    semaphoreOperation(lobby->semID, SEM_P);
}

void leaveLobbyMemory(Lobby *lobby)
{
    semaphoreOperation(lobby->semID, SEM_V);
}


void semaphoreOperation(int semId, int operation) {
    semaphore.sem_op = operation;
    semop(semId, &semaphore, 1);
}

void spreadChatMessage(int senderPID, int playersShmID)
{
    ClientInfo *clients = shmat(playersShmID, NULL, 0);
    for(int i=0;i<MAX_PLAYER_NUMBER;i++, clients++)
    {
        printf("%d\n" , clients->PID);
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