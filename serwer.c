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
void findNewClients();
void addNewClient(InitialMessage *newClient, ClientInfo *clientPIDSArray, int *clientsArrayIndex);
int findClientByPID(int clientPID, ClientInfo *clientsArray); //return index of a client with PID in array of clientInfo structures

    ///SHARED MEMORY
int initializePlayers();    //returns id to shm
int initializeRooms(Room *rooms);      //returns id to shm
char *RoomsToMessage();


    ///SEMAPHORES
int roomLock();             //group of sem to rooms
int playersLimitLock();     //returns id to lock sem of max players


//globals
int currentNumberOfClients_GLOBAL = 0;
int serverPID ;

struct sembuf semaphore;

int main(int argc, char * argv [])
{
    Room rooms[MAX_ROOMS_NUMBER];
    Lobby lobby;


    //semafory:
    /*
        pola z planszą dla każdego z pokojow
        liczba graczy




    */

    /*
    shared memory
        kazda z plansz
        pokoje


    */

    lobby.semID =  semget(LOBBY_SEMAPHORE_KEY, 1, IPC_CREAT | DEFAULT_RIGHTS);
    lobby.shmID = shmget(LOBBY_MEMORY_KEY, sizeof(lobby) + sizeof(rooms), IPC_CREAT | DEFAULT_RIGHTS); 

    serverPID = getpid();
    int roomsShmID = initializeRooms(rooms);
    int playersShmID = initializePlayers();
    
    lobby.rooms = rooms;

    int forked = fork();
    if(forked == 0)
    {
        //child's process
        //initialKey used in the message queue that handles new clients
        findNewClients();

    }
    else if (forked > 0)
    {
        //nothing yet
    }
    else
    {
        perror("An error occured when forking in main server: ");
    }

    return 0;


}


void findNewClients()
{
        //initialization of messages structures
    InitialMessage  message2Rcv;
    
    ClientInfo clientsPIDs[MAX_PLAYER_NUMBER];
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

            addNewClient(&message2Rcv, clientsPIDs, &currentNumberOfClients);



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
                            printf("Sent private message Successfully \n");
                    }
                    

                    resetPrivateMessageStructure(&newPrivateMessage);
                    
                    prepareLobbyDataToSend(&newPrivateMessage);
                    strcpy(newPrivateMessage.content, "ad");
                    newPrivateMessage.type = GAME_SERVER_TO_CLIENT;

                   
                    if(sendPrivateMessage(privateMessageID, &newPrivateMessage) == -1)
                    {
                    printf("Error asdsadwhen sending private message to the client!\n");
                    }
                    else
                    {
                        if(debug)
                            printf("Sentasdsada private message Successfully \n");
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

void addNewClient(InitialMessage *newClient, ClientInfo clientPIDSArray[], int *clientsArrayIndex)
{
    ClientInfo newClientInfo;
    newClientInfo.PID = newClient->pid;
    newClientInfo.lobbyIndex = -1;
    strcpy(newClientInfo.nickname, newClient->username);
    clientPIDSArray[(*clientsArrayIndex)++] = newClientInfo;
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




///FUNCTIONS FOR SHARED MEMORY
int initializePlayers()
{
    int memoryID = shmget(PLAYERS_STRUCTURE_KEY, sizeof(ClientInfo) * MAX_PLAYER_NUMBER, IPC_CREAT | DEFAULT_RIGHTS);
    if(memoryID == -1)
    {
        perror("Failed to initialize players memory ");
        return -1;
    }
    return memoryID;

}   //returns id to shm
int initializeRooms(Room *rooms)
{
    int memoryID = shmget(ROOMS_STRUCTURE_KEY, sizeof(Room) * MAX_ROOMS_NUMBER, IPC_CREAT | DEFAULT_RIGHTS);
    if(memoryID == -1)
    {
        perror("Failed to initialize rooms memory ");
        return -1;
    }
    Room *roomIter=rooms;
    for(int i=0;i<MAX_ROOMS_NUMBER;i++, roomIter++)
    {
        roomIter->state = ROOM_EMPTY;
    }

    return memoryID;
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

char * prepareLobbyDataToSend(Lobby *lobby)
{
    enterLobbyMemory(lobby);

   


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


void semaphoreOperation(int semId, int operation) {
    semaphore.sem_op = operation;
    semop(semId, &semaphore, 1);
}