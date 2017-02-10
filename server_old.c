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
#include "structures.h"

int main(int argc, char * argv [])
{
	
	
	//initialization of messages structures
	InitialMessage message2Send, message2Rcv;
	message2Send.mtype = 1;
	memset(message2Send.mtext, '0',initialMessageSize);
	
//initialKey used in the message queue that handles new clients
	int initialMessageKey = 999;

	int clientsPIDs[maxClients] = {0};

	int initialMessageId = msgget(initialMessageKey,IPC_CREAT | 0666);
	int b = msgctl(initialMessageId,IPC_RMID, NULL);
	if(b == -1)
	{
		perror("error with msgctl: 0");
		exit(0);
	}
			perror("error with msgctl: 0");


	if(initialMessageId == -1)
	{
		perror("sd ");
		if(errno == EEXIST)
		{
			printf("Queue already exists\n");
			msgctl(initialMessageId, IPC_RMID,0);

			//initialMessageId = msgget(initialMessageId, IPC_CREAT | IPC_EXCL | 0666);
			if(initialMessageId == -1)
			{
				perror("error: ");
			}
			printf("%d", initialMessageId);
			exit(0);
		}
		else
		{
			perror("Failed to start messages queue for clients");
			exit(0);	
		}
	}


	if(fork() == 0)
	{
		//child's process
		//listening for new clients

				//int receivedDataSize = msgrcv(initialMessageId, &message2Rcv, initialMessageSize,2,2);
	while(true)
	{				printf("New client messageasda %s %d\n", message2Rcv.mtext, message2Rcv.mtype);
	}
				//printf("New client message %s , its pid as an int: %d\n", message2Rcv.mtext, message2Rcv.clientsPID);


	}
	else
	{
		//parent's process
		/*
			if(msgsnd(initialMessageId, &message2Send, initialMessageSize,0) == 0)
		
	{
		printf("Sending data successfully\n");
	}
*/	int a = msgctl(initialMessageId, IPC_RMID ,0);
	if(a == -1)
		perror("blad z msgctl ");
	else
		printf("Msgctl removed msg queue");
		
	}
	int a = msgctl(initialMessageId, IPC_RMID ,0);
	if(a == -1)
		perror("blad z msgctl ");
	else
		printf("Msgctl removed msg queue");
	return 0;
}
