CC=gcc
CFLAGS = -Wall \
	 -Werror

all: ../client

.SUFFIXES:
	MAKEFLAGS += -r

../client: client.o
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo [LD] $@

%.o: %.c 
	@$(CC) $(CFLAGS) -c -o $@ $^
	@echo [CC] $@
