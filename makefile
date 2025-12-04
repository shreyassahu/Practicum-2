all: server client

server: server.c
	gcc server.c -o rfserver

client: client.c
	gcc client.c -o rfs
