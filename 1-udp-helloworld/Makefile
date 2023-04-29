client.o: client.c
	gcc -Wall -g -c client.c

server.o: server.c
	gcc -Wall -g -c server.c

client: client.o
	gcc -Wall -g -o client client.o

server: server.o
	gcc -Wall -g -o server server.o

clean:
	rm -f *.o client server

