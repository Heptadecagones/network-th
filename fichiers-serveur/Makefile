CC=gcc
CFLAGS= -Wall \
	-Werror
LDFLAGS=-lsqlite3
cyan = /bin/echo -e "\x1b[36m\#\# $1\x1b[0m"

all: ../server

.SUFFIXES:
	MAKEFLAGS += -r

../server: server.o dbutil.o commands.o
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo [LD] $@

%.o: %.c 
	@$(CC) $(CFLAGS) -c -o $@ $^
	@echo [CC] $@
