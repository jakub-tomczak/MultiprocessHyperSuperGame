#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
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
int getMessageQueue(int key);   //get - create message queue represented by the key
//globals
int currentNumberOfClients_GLOBAL = 0;


int main(int argc, char * argv [])
{
    int shmSemID = semget();
    int forked = fork();
    if(forked == 0)
    {
        //child's process
        //initialKey used in the message queue that handles new clients
        printf("pid of while: %d\n", getpid());
        findNewClients();

    }
    else if (forked > 0)
    {
        //chat handling
        printf("pid of forked > 0: %d\n", getpid());

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
    InitialMessage message2Send, message2Rcv;

    message2Send.mtype = 1;
    memset(message2Send.clientsName, '0',INITIAL_MESSAGE_SIZE);
    
    ClientInfo clientsPIDs[MAX_CLIENTS_NUMBER + 1];
    int currentNumberOfClients = 0;

    int initialMessageId = msgget(INITIAL_MESSAGE_KEY, IPC_CREAT | IPC_EXCL | MESSAGE_QUEUE_RIGHTS);
    if(initialMessageId == -1)
    {
        if(errno == EEXIST)
        {
            if(debug)
                printf("Queue already exists\n");
            initialMessageId = msgget(INITIAL_MESSAGE_KEY, MESSAGE_QUEUE_RIGHTS);
        }
        else
        {
            perror("error getting message queue in findNewClients serwer : ");
            exit(0);
        }
    }

    while(true)
    {
        if(currentNumberOfClients == MAX_CLIENTS_NUMBER)
        {
            printf("Max number of clients exceded!\n");
            //tutaj walnąć semafor który zamkniemy, usuniecie klienta spowoduje podniesienie semafora  v  ;
        }
        message2Rcv.mClientsPID = -1;
        int recivedMessage =  msgrcv(initialMessageId,&message2Rcv, INITIAL_MESSAGE_SIZE, 2, 0);
        if(recivedMessage == -1)
        {

            perror("Error after reciving a message:");
            break;
        }
        else
        {
            int a;
            //new client hass been found!
            if(debug)
                printf("Recived a message, %i_%d_sizeof_%d\n", message2Rcv.mClientsPID, INITIAL_MESSAGE_SIZE, recivedMessage);
            
            addNewClient(&message2Rcv, clientsPIDs, &currentNumberOfClients);

            if(fork() == 0)
            {
                //childs process - keep private communication with a client in that process
                int privateMessageID = getMessageQueue(message2Rcv.mClientsPID);
                if(privateMessageID == -1)
                {
                    printf("Couldn't launch message queue for client with PID: %d, exiting...\n", message2Rcv.mClientsPID);
                    break;
                }

                resetInitialMessageStructure(&message2Rcv); //clears message2Rcv

                
            }




        }

    }

    //delete message queue after the error
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

void addNewClient(InitialMessage *newClient, ClientInfo clientPIDSArray[], int *clientsArrayIndex)
{
    ClientInfo newClientInfo;
    newClientInfo.PID = newClient->mClientsPID;
    newClientInfo.lobbyIndex = -1;
    strcpy(newClientInfo.nickname, newClient->clientsName);
    clientPIDSArray[(*clientsArrayIndex)++] = newClientInfo;
    currentNumberOfClients_GLOBAL = clientsArrayIndex;
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


int getMessageQueue(int key)
{
    int initialMessageId = msgget(key, IPC_CREAT | IPC_EXCL | MESSAGE_QUEUE_RIGHTS);
    if(initialMessageId == -1)
    {
        if(errno == EEXIST)
        {
            if(debug)
                printf("Queue already exists\n");
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