CFLAGS=-std=c99 -Wall -Wextra -pedantic -g
BIN=test_parser
CC=gcc
RM=rm -f

ALL: str.o test_scanner.o test_error.o test_parser.o
	$(CC) $(CFLAGS) -o $(BIN) str.o test_scanner.o test_error.o test_parser.o

clean:
	$(RM) *.o $(BIN)

