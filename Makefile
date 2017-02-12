all: client server

client: client.c
	gcc -o client.out client.c -Wall
server: serwer.c
	gcc -o server.out serwer.c