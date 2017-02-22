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

int initializeChat(int parentPID);
void displayGameboard(GameMatrix *gameboard);
void semaphoreOperation(int semId, int operation);
bool serverAvailable = true;
struct sembuf semaphore;

int main(int argc, char * argv[])
{
	char username[USER_NAME_LENGTH];
	printf("Wpisz swoja nazwe uzytkownika: ");
	scanf("%49s", username);

	printf("Witaj, %s\n", username);


	InitialMessage message2Snd;
	message2Snd.type = GAME_CLIENT_TO_SERVER;
	int myPID = getpid();
	int room = -1;

	int forked = fork();
	if(forked == 0)
	{
		char myPID_char[5];
		sprintf(myPID_char, "%d", myPID);
		printf("Opening chat with pid %d\n", getpid());
		
//		if(chatListenerFork == 0)
//		{
			//execl("/usr/bin/gnome-terminal", "gnome-terminal" , "-x" ,"./chat.out", myPID_char, username,NULL);
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
		int status;
		printf("waiting for pid = %d\n", forked);
		pid_t result = waitpid(forked, NULL, WNOHANG);
		if (result == 0) {
			printf("Doing normal job\n");
		//do normal work
		} else if (result == -1) {
  			// Error 
		} else {
  			printf("closed\n");
  			exit(0);
		}

		int privateMessageID = getMessageQueue(myPID);

		strcpy(message2Snd.username,username);
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
				perror("Blad podczas odbioru pid serwera! ");
				exit(0);
			}
			else
			{
				printf("Server pid:%s\n", newPrivateMessage.content);
			}
			resetPrivateMessageStructure(&newPrivateMessage);
			if(receivePrivateMessage(privateMessageID, &newPrivateMessage ,GAME_SERVER_TO_CLIENT) == -1)
			{
				perror("Blad podczas odbioru zawartosci lobby z serwera! ");
				exit(0);
			}
			else
			{
				printf("Wybierz pokoj:\n%s\n", newPrivateMessage.content);
			}



			int serversresponse = 0;
			do
			{
				int roomIndex = -1;
				printf("Wybierz nr pokoju: ");
				scanf("%d", &roomIndex);

				
				serversresponse = -1;
				resetPrivateMessageStructure(&newPrivateMessage);
				newPrivateMessage.type = GAME_CLIENT_TO_SERVER;
				sprintf(newPrivateMessage.content, "%d", roomIndex);


				if(sendPrivateMessage(privateMessageID, &newPrivateMessage) == -1)
				{
					printf("Failed to send information with room index to the server!\n");
				}
				if(debug)
					printf("Send room choose message succesfully!\n");
			

				resetPrivateMessageStructure( &newPrivateMessage);
				newPrivateMessage.type = GAME_SERVER_TO_CLIENT;
				                            printf("reset content %s\n", newPrivateMessage.content );
				if(receivePrivateMessage(privateMessageID, &newPrivateMessage ,GAME_SERVER_TO_CLIENT) == -1)
				{
					perror("Blad podczas odbioru Wiadomosci z serwera! ");
					exit(0);
				}
				else
				{
					//printf("Odebrane dane: %s\n", newPrivateMessage.content);
					if (isdigit(newPrivateMessage.content[0])) {
						serversresponse = atoi(newPrivateMessage.content);
						room = roomIndex;
						printf("Dodano do pokoju nr %d, serversresponse: %d\n", room);
						break;
					}
					else
					{
						printf("Nie zostales dodany do pokoju! \n");
						serversresponse = -1;
					}
				}
			}while(serversresponse == -1);
			
			if(room == -1) 
				{
					exit(0);
				}

			int gameKey = room +50;

			GameMatrix gameMatrix;
			//set semaphore
    		gameMatrix.semID = semget(gameKey, 1, IPC_CREAT | DEFAULT_RIGHTS);
    		semctl(gameMatrix.semID,0, SETVAL, 1);
    		//set shared memory
    		gameMatrix.memID = shmget(gameKey, GAME_MATRIX_CELLS, IPC_CREAT | DEFAULT_RIGHTS);
    		gameMatrix.board = shmat(gameMatrix.memID, 0, 0);
			//wait for a move
			PrivateMessage gameMessage;
			resetPrivateMessageStructure(&gameMessage);
			gameMessage.type = GAME_SERVER_TO_CLIENT;
	
			int gameState = 1;
			do
			{
				printf("Oczekiwanie na rozpoczęcie gry\n ", privateMessageID);
				if(receivePrivateMessage(privateMessageID, &gameMessage,GAME_SERVER_TO_CLIENT) == -1)
					{
						printf("Connection broken!\n");
						msgctl(privateMessageID, IPC_RMID, NULL);
						semctl(gameMatrix.semID, 0, IPC_RMID, NULL);
						shmctl(gameMatrix.memID, IPC_RMID, NULL);
						gameState = -1;
						break;
					}
				//printf("received private message >%s<\n", gameMessage.content);
				gameMessage.type = GAME_CLIENT_TO_SERVER;

				if(isdigit(gameMessage.content[0]))
				{
					int gameResponse = atoi(gameMessage.content);
					//printf("response after atoi>%d<\n", gameResponse);
					switch(gameResponse)
					{
						case 0:
						case 2:
							if(gameResponse == 0)
								printf("Ponow ruch\n");
							else
								printf("Twój ruch\n");
							displayGameboard(&gameMatrix);
							scanf("%s", gameMessage.content);
							sendPrivateMessage(privateMessageID, &gameMessage);
							gameMessage.type = GAME_SERVER_TO_CLIENT;
							break;
						case 1:
							printf("Ruch zaakceptowany\n");
							break;
						default:
							printf("Message from server: %s\n",gameMessage.content);
							break;
					}
				}
				else
				{
					printf("Koniec gry! [%s]\n", gameMessage.content);
				}
							//printf("Message from server: %s\n",gameMessage.content);

				

	
			} while(gameState > -1);
			printf("Exitied\n");
	}


	return 0;




}

void displayGameboard(GameMatrix *gameboard)
{
	semaphoreOperation(gameboard->semID, SEM_P);

	char *pointer = gameboard->board;
	for(int i=0;i<GAME_MATRIX_CELLS;i++, pointer++)
	{
		if(i%5 == 0 && i > 0)
		{
			printf("|\n");
		}
		printf("|%c", *pointer);
	}
	printf("|\n");

	semaphoreOperation(gameboard->semID, SEM_V);
}

void semaphoreOperation(int semId, int operation) {
    semaphore.sem_op = operation;
    semop(semId, &semaphore, 1);
}



int initializeChat(int parentPID)
{
	int mesageID = getMessageQueue(CHAT_MESSAGE_KEY);

}
