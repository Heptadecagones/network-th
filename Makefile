CC=gcc

all: client server

.SUFFIXES:
	MAKEFLAGS += -r

client:
	@$(MAKE) -C Client

server:
	@$(MAKE) -C Serveur

clean:
	rm -r client server **/*.o
