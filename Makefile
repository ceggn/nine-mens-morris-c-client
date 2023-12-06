CC = gcc
CFLAGS = -Wall -Werror -Wextra -g -std=c99
TARGETS = sysprak-client performConnection sharedMemory config think 
SRCFILES = $(addsuffix .c, $(TARGETS))
CONF = client.conf clientTest.conf

all: sysprak-client.o performConnection.o config.o sharedMemory.o think.o
	$(CC) $(CFLAGS) -o sysprak-client sysprak-client.o performConnection.o config.o sharedMemory.o think.o

clean:
	rm -f $(TARGETS)
	rm -f $(addsuffix .o, $(TARGETS))
	rm -f $(addsuffix .h.gch, $(TARGETS))

sysprak-client.o: sysprak-client.c
	$(CC) $(CFLAGS) -c sysprak-client.c

performConnection.o: performConnection.c
	$(CC) $(CFLAGS) -c performConnection.c

config.o: config.c
	$(CC) $(CFLAGS) -c config.c

sharedMemory.o: sharedMemory.c
	$(CC) $(CFLAGS) -c sharedMemory.c

think.o: think.c
	$(CC) $(CFLAGS) -c think.c

play: all
	sysprak-client -g $(GAME_ID) -p $(PLAYER) #-c clientTest.conf

test: all
	sysprak-client -g 3os6vx4lthb2v -p 1

test2: all
	sysprak-client -g 3os6vx4lthb2v -p 2

mem:
	valgrind --show-leak-kinds=all -q --trace-children=yes --leak-check=full sysprak-client -g 3os6vx4lthb2v -p 1 

zip:
	zip sysprakmuehle.zip $(SRCFILES) $(addsuffix .h, $(TARGETS)) $(CONF) Makefile

points:
	rm -f -r build
	chmod +x sysprak-abgabe.sh
	./sysprak-abgabe.sh build log sysprakmuehle.zip --spectate


