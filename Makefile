
all: client server

client:
	gcc -c ./Client/client.c

server:
	gcc -c ./Serveur/server.c

clean:
	rm *.o
	rm client
	rm server
