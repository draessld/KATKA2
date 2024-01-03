SHELL = /bin/sh
CC = g++
CFLAGS = -Wall -Wextra -pedantic -Wshadow -funroll-loops -O3 -DNDEBUG -march=native -std=c++0x
#CFLAGS = -Wall -Wextra -pedantic -Wshadow -g2

all: test

test:
	$(CC) $(CFLAGS) -o generate_tree generate_tree.cpp

clean:
	/bin/rm -f *.o

nuclear:
	/bin/rm -f generate_tree *.o
