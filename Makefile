
all: client server
	gcc -o client client2.o
	gcc -o server server2.o

client:
	gcc -c ./Client/client2.c

server:
	gcc -c ./Serveur/server2.c

clean:
	rm *.o
	rm client
	rm server
