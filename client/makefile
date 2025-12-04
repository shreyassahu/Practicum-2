CC = gcc
CFLAGS = -Wall

all: rfs

rfs: client.c client_handlers.c
	$(CC) $(CFLAGS) -o rfs client.c client_handlers.c

clean:
	rm -f rfs