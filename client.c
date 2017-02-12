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
#include <stdlib.h>
#include <errno.h>	//errno
#include "structures.h"


int main(int argc, char * argv[])
{
	InitialMessage message2Snd, message2Rcv;
	message2Snd.mtype = 2;
	int myPID = getpid();


//strcpy(message.mtext, myPidKey);
	sprintf(message2Snd.mtext, "%d", myPID);
	message2Snd.mClientsPID = myPID;

	int initialMessageId = msgget(INITIAL_MESSAGE_KEY, MESSAGE_QUEUE_RIGHTS);
	if(initialMessageId == -1)
	{
			perror("Failed to find the server");
			exit(0);
	}

	printf("Sending data to the server\n");
	//we can send the message to the server
	if(msgsnd(initialMessageId, &message2Snd, initialMessageSize, 0) == -1)
	{
		perror("Failed to send initial message to the server!");
		exit(0);
	}
	else
	{
		printf("1.Sent initial message to the server\n");
	}
	memset(message2Rcv.mtext, '0', initialMessageSize);


	int recivedMessageSize = msgrcv(initialMessageId, &message2Rcv, initialMessageSize, 1,0);

	printf("2.Recived data from a server %s\n", message2Rcv.mtext);


	msgctl(initialMessageId, IPC_RMID,0);
	return 0;




}

