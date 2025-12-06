CC = gcc
CFLAGS = -Wall -pthread

all: rfserver

rfserver: server.c server_handlers.c
	$(CC) $(CFLAGS) -o rfserver server.c server_handlers.c

clean:
	rm -f rfserver 
	rm -rf data_files