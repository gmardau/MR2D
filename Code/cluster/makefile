CC = g++
-CC = -Wall -ansi -pedantic -lm -pthread -O3 -static -static-libgcc -static-libstdc++ -std=c++14

default: example

example: example.cpp
	$(CC) $(-CC) example.cpp -o example

all: example

clean:
	rm -f example

.PHONY: example
