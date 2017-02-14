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
#include <stdbool.h> //bool expressions



int initializeMessageQueue(int key);

int main(int argc, char const *argv[]) {
	int parentPid = atoi(argv[1]);
        
	int messageID = initializeMessageQueue(parentPid);

	if(messageID == -1)
	{
		printf("Could not connect with the server, exiting...\n" );
		exit(0);
	}
	bool continueWorking = true;
	bool queueExists = true;
	int forked = fork();
	if(forked == 0)
	{	//child's process waits for the exit of 
		int status;
		pid_t result = waitpid(parentPid, &status, WNOHANG);
		if (result == 0) {
		//do normal work
		} else if (result == -1) {
  			// Error 
		} else {
  			continueWorking = false;
  			exit(0);
		}
	}
	else if(forked > 0)
	{

			int forked2 = fork();
			if(forked2 == 0)
			{	//sending functionalities
				while(continueWorking)
				{
					ChatMessage chatMessage;
					int receivedMessage = receiveChatMessage(messageID, &chatMessage, CHAT_SERVER_TO_CLIENT);
					if(receivedMessage != -1)
					{
						printf("%s napisal(a): %s\n\n",chatMessage.source, chatMessage.content);
					}
				}

			}
			else
			{	//receiving functionalities
				while(continueWorking)
				{
					ChatMessage chatMessage;
					chatMessage.type = CHAT_CLIENT_TO_SERVER;
					printf("Podaj wiadomosc: ");
					char messageContent[MESSAGE_CONTENT_SIZE];
					scanf("%512s", messageContent);
					int sentMessage = sendChatMessage(messageID, &chatMessage);
					if(sentMessage == -1)
					{
						printf("Wysylanie wiadomosci sie nie powiodlo!\n");
					}
				}

			}
		
		if(queueExists)
		{
			msgctl(messageID, IPC_RMID, NULL);
		}
		queueExists = false;
		exit(0);
	}

return 0;
}

int initializeMessageQueue(int key)
{
	int messageQ = getMessageQueue(key);
	if(messageQ == -1)
	{
		printf("Failed to connect with the server!");
		return -1;
	}
	return messageQ;
}
