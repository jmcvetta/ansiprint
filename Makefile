CC=g++
CGLAGS=-Wall -O2
all: ansiprint

ansiprint: ansiprint.cc
	$(CC) $(CGLAGS) -o ansiprint ansiprint.cc

install:
	cp ansiprint /usr/local/bin

clean: 
	rm -f ansiprint

uninstall:
	rm /usr/local/bin/ansiprint
