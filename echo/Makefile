CC = gcc
CFLAGS = -O2 -Wall -I .
LIB = -lpthread

all: echo

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

echo-client: echo-client.c csapp.o
	$(CC) $(CFLAGS) -o echo-client echo-client.c csapp.o $(LIB)

echo-server: echo-server.c csapp.o
	$(CC) $(CFLAGS) -o echo-server echo-server.c csapp.o $(LIB)

clean:
	rm -f *.o echo *~
	rm -f *.o echo-client *~
	rm -f *.o echo-server *~
