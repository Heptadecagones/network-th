CC=gcc
CFLAGS= -Wall \
	-Werror
LDFLAGS=

all: client server

%: %.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

client.o:
	gcc -c ./Client/client.c

server.o:
	gcc -c ./Serveur/server.c

clean:
	rm *.o
	rm client
	rm server
