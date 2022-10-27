CC=gcc

all: client server

.SUFFIXES:
	MAKEFLAGS += -r

.PHONY: client server

client:
	@$(MAKE) -C fichiers-client

server:
	@$(MAKE) -C fichiers-serveur

clean:
	rm -r client server **/*.o
