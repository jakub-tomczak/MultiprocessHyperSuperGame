CFLAGS=-Wall
LFLAGS=-std=c99
CC=gcc
all: client server chat

client: client.c
	$(CC) $(CFLAGS) $(LFLAGS) -o client.out client.c -Wall
server: serwer.c
	$(CC) $(CFLAGS) $(LFLAGS) -o server.out serwer.c
chat: chat.c
	$(CC) $(CFLAGS) $(LFLAGS) -o chat.out chat.c