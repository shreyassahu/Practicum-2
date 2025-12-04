all: client

client: client.o write.o get.o ls.o rm.o utilities.o
		gcc -o client client.o write.o get.o ls.o rm.o utilities.o

client.o: client.c write.h get.h ls.h rm.h utilities.h
		gcc -c client.c

write.o: write.c 
		gcc -c write.c

get.o: get.c utilities.h
	gcc -c get.c

ls.o: ls.c
	gcc -c ls.c

rm.o: rm.c
	gcc -c rm.c

utilities.o: utilities.c
			gcc -c utilities.c

clean: 
	rm -f *.o client *.txt