CC=gcc
FLAGS=

ALL: server client

server: $(DEPS) account.h
	$(CC) bankserver.c -o server $(FLAGS);
	
client: $(DEPS) account.h
	$(CC) bankclient.c -o client $(FLAGS)	

libsl: account.h bankserver.c bankclient.c
	ar rcs libsl.a account.h bankserver.c bankclient.c

clean:
	rm server client

