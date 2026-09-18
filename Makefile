CC = gcc
CFLAGS = -Wall -g

all: dserver dclient

dserver: server.o
	$(CC) $(CFLAGS) -o dserver server.o

dclient: client.o
	$(CC) $(CFLAGS) -o dclient client.o

server.o: server.c 

client.o: client.c 

clean:
	rm -f -r *.dSYM fifo* dserver dclient *.o request response_*

cleanFiles:
	rm *.dat
