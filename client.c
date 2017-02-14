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
	char username[USER_NAME_LENGTH];
	printf("Wpisz swoja nazwe uzytkownika: ");
	scanf("%49s", username);

	printf("Witaj, %s\n", username);


	InitialMessage message2Snd;
	message2Snd.type = GAME_CLIENT_TO_SERVER;
	int myPID = getpid();
	int privateMessageID = getMessageQueue(myPID);

	sprintf(message2Snd.username, "%d", myPID);
	message2Snd.pid = myPID;

	int initialMessageId = msgget(INITIAL_MESSAGE_KEY, DEFAULT_RIGHTS);
	if(initialMessageId == -1)
	{
			perror("Failed to find the server");
			exit(0);
	}

	printf("Connecting with the server\n");
	//we can send the message to the server
	if(msgsnd(initialMessageId, &message2Snd, sizeof(message2Snd) - sizeof(message2Snd.type), 0) == -1)
	{
		perror("Failed to send initial message to the server!");
		exit(0);
	}
	else
	{
		printf("1.Sent initial message to the server\n");
	}


	if(privateMessageID == -1) 
	{
		printf("Nie mozna znalezc/stworzyc prywatnej kolejki dla tego klienta\n");
	}
	else
	{
		if(debug){
			printf("Got private message queue %d\n", privateMessageID);
		}
	}
	PrivateMessage newPrivateMessage;	
	if(receivePrivateMessage(privateMessageID, &newPrivateMessage ,0) == -1)
	{
		perror("Blad podczas odbioru Wiadomosci z serwera! ");
		exit(0);
	}




	//int recivedMessageSize = msgrcv(initialMessageId, &message2Rcv, INITIAL_MESSAGE_SIZE, 1,0);

	//printf("2.Recived data from a server %s\n", message2Rcv.clientsName);


	//msgctl(initialMessageId, IPC_RMID,0);
	return 0;




}

