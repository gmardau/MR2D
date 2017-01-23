CC = gcc

CFLAGS  = -Wall -ansi -lm -g -O3 -static -static-libgcc

HEADERS = *.h
SOURCES = *.c *.cpp

build: $(HEADERS) $(SOURCES)
	clear
	$(CC) $(SOURCES) $(CFLAGS) -o main

clean:
	clear
	rm main
