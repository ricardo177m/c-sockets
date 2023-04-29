all: client server

.c.o:
	gcc -Wall -g -c $<

client: client.o
	gcc -Wall -g -o client client.o

server: server.o
	gcc -Wall -g -o server server.o

clean:
	rm -f *.o client server

.DEFAULT_GOAL := all

