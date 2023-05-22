CC = gcc
CFLAGS = -O2 -Wall -I .
LIB = -lpthread

all: socket

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

socket: host-info.c csapp.o
	$(CC) $(CFLAGS) -o host-info host-info.c csapp.o $(LIB)

clean:
	rm -f *.o host-info *~
