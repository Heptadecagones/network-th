CC=gcc

all: client server

.SUFFIXES:
	MAKEFLAGS += -r

.PHONY: client server

client:
	@$(MAKE) -C Client

server:
	@$(MAKE) -C Serveur

clean:
	rm -r client server **/*.o
