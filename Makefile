CC=g++
CGLAGS=-Wall -O2 -mpentium
all: ansiprint

ansiprint: ansiprint.cc
	$(CC) $(CGLAGS) -o ansiprint ansiprint.cc

install:
	cp ansiprint /usr/local/bin

clean: 
	rm -f test ansiprint

uninstall:
	rm /usr/local/bin/ansiprint
