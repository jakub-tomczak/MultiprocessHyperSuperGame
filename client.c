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
#include <stdbool.h>

int initializeChat(int parentPID);

bool serverAvailable = true;

int main(int argc, char * argv[])
{
	char username[USER_NAME_LENGTH];
	printf("Wpisz swoja nazwe uzytkownika: ");
	scanf("%49s", username);

	printf("Witaj, %s\n", username);


	InitialMessage message2Snd;
	message2Snd.type = GAME_CLIENT_TO_SERVER;
	int myPID = getpid();

	int forked = fork();
	if(forked == 0)
	{
		char myPID_char[5];
		sprintf(myPID_char, "%d", myPID);
		
//		if(chatListenerFork == 0)
//		{
			execl("/usr/bin/gnome-terminal", "gnome-terminal" , "-x" ,"./chat.out", myPID_char, username,NULL);
			perror("Failed to open chat window: ");
/*
		}
		else if(chatListenerFork > 0)
		{	//creating chat message queue or connecting to the existing one
			int chatID= initializeChat(myPID);
			if(chatID != -1)
			{
				printf("Connected with chat client!\n");
			}
		}
*/		


	}
	else
	{
		int privateMessageID = getMessageQueue(myPID);

		sprintf(message2Snd.username, "%d", myPID);
		message2Snd.pid = myPID;
		message2Snd.type = GAME_CLIENT_TO_SERVER;

	int initialMessageId = getMessageQueue(INITIAL_MESSAGE_KEY);
	if(initialMessageId == -1)
	{
			perror("Failed to find the server");
			serverAvailable =false;
			exit(0);
	}
	printf("Connected with server\n");
	//we can send the message to the server
	int msgSnd = sendInitialMessage(initialMessageId, &message2Snd);
	if(msgSnd == -1)
	{
		perror("Failed to send initial message to the server!");
		exit(0);
	}
	else
	{
		printf("Sent a  %d\n", message2Snd.type);
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
		if(receivePrivateMessage(privateMessageID, &newPrivateMessage ,GAME_SERVER_TO_CLIENT) == -1)
		{
			perror("Blad podczas odbioru Wiadomosci z serwera! ");
			exit(0);
		}
		else
		{
			printf("Server pid:%s\n", newPrivateMessage.content);
		}
		resetPrivateMessageStructure( &newPrivateMessage);
		if(receivePrivateMessage(privateMessageID, &newPrivateMessage ,GAME_SERVER_TO_CLIENT) == -1)
		{
			perror("Blad podczas odbioru Wiadomosci z serwera! ");
			exit(0);
		}
		else
		{
			printf("2Server pid:%s\n", newPrivateMessage.content);
		}

	}


	return 0;




}

int initializeChat(int parentPID)
{
	int mesageID = getMessageQueue(CHAT_MESSAGE_KEY);

}
