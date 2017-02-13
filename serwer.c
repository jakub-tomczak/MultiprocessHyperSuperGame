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
void addNewClient();

int main(int argc, char * argv [])
{
    int shmSemID = semget();
    int forked = fork();
    if(forked == 0)
    {
        //child's process
        //initialKey used in the message queue that handles new clients
        findNewClients();
    }
    else if (forked > 0)
    {
        //chat handling

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
    
    unsigned short clientsPIDs[MAX_CLIENTS_NUMBER + 1];
    memset(clientsPIDs, 0, MAX_CLIENTS_NUMBER + 1); //set all table indexes to 0
    
    int initialMessageId = msgget(INITIAL_MESSAGE_KEY, IPC_CREAT | IPC_EXCL | MESSAGE_QUEUE_RIGHTS);
    int currentNumberOfClients = 0;
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
        printf("%d -> message id\n", initialMessageId);

        int recivedMessage =  msgrcv(initialMessageId,&message2Rcv, INITIAL_MESSAGE_SIZE, 2, 0);
        if(recivedMessage == -1)
        {

            perror("Error after reciving a message:");
            break;
        }
        else
        {
            //new client hass been found!
            if(debug)
                printf("Recived a message, %d\n", message2Rcv.mClientsPID);

            addNewClient(message2Rcv.mClientsPID);
            if(!fork())
            {
                //childs process - keep private communication with a client in that process

            }


        }

        printf("2 %d -> message id\n", initialMessageId);



        int responseMessage = msgsnd(initialMessageId, &message2Send, INITIAL_MESSAGE_SIZE, 0);
        if(responseMessage == -1)
        {
            perror("Error while responding to the client in findNewClients serwer: ");
        }
        else
        {
            if(debug)
                printf("Successfully sent data to the new client in findNewClients serwer\n");
        }

        ///wait for a second before next checking
        sleep(1);
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
/*
bool addNewClient(int newClientPID, )
{

}
*/
