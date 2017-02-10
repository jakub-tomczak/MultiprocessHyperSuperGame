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

void displayStats( int initialMessageId)
{
	msqid_ds infoStruct;
	int recived = msgctl(initialMessageId, IPC_STAT, &infoStruct);
	if(recived == -1)
	{
		perror("Error when obtaining queue info: ");
	} 
	else
	{
		printf("%d, \n", infoStruct.msg_qnum);
	}
}

int main(int argc, char * argv [])
{
	
	
	//initialization of messages structures
	InitialMessage message2Send, message2Rcv;

	message2Send.mtype = 1;
	memset(message2Send.mtext, 'n',initialMessageSize);
	
	int forked = fork();
	if(forked == 0)
	{
		//child's process
		//initialKey used in the message queue that handles new clients
		

		int clientsPIDs[maxClients] = {0};
		int initialMessageId = msgget(initialMessageKey, IPC_CREAT | IPC_EXCL | 0777);
		if(initialMessageId == -1)
		{
			perror("error getting message queue: ");
			exit(0);
		}

		while(true)
		{

			displayStats(initialMessageId);

			printf("%d -> message id\n", initialMessageId);

			int recivedMessage =  msgrcv(initialMessageId,&message2Rcv, initialMessageSize, 2, 0);
			if(recivedMessage == -1)
			{
				
				perror("Error after reciving a message:");
				break;
			}
			else
			{
				printf("Recived a message, %d\n", message2Rcv.mClientsPID);
				
			}

						printf("2 %d -> message id\n", initialMessageId);



			int responseMessage = msgsnd(initialMessageId, &message2Send, initialMessageSize, 0);
			if(responseMessage == -1)
			{
				perror("Error while responding to the client: ");
				break;
			}
			else
			{
				printf("Successfully sent data to the new client\n");
			}
		}
		
		//delete message queue after the error
		int check = msgctl(initialMessageId, IPC_RMID, NULL);
		if(check == -1)
			{
				perror("Msgctl error");
			}
		else
			{
				printf("Successfully deleted a queue\n");
			}
			
			return 0;
	}
	else if (forked > 0)
	{
		printf("Server's normal work\n");
	}
	else
	{
		perror("An error occured: ");
	}
	




	return 0;


}
