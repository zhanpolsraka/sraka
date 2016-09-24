CFLAGS=-std=c99 -Wall -Wextra -pedantic -g
BIN=test_scanner
CC=gcc
RM=rm -f

ALL: 
	$(CC) $(CFLAGS) -o $(BIN) test_scanner.c

clean:
	$(RM) *.o $(BIN)
