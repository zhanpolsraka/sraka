CFLAGS=-std=c11 -Wall -Wextra -pedantic -g

BIN=ifj16

CC=gcc
RM=rm -f

ALL: str.o scanner.o error.o buffer.o precedence.o  table.o instructions.o parser.o frame.o interpret.o built_in.o main.o
	$(CC) $(CFLAGS) -o $(BIN) str.o scanner.o error.o buffer.o precedence.o table.o instructions.o parser.o frame.o interpret.o built_in.o main.o
run:
	./$(BIN)

clean:
	$(RM) *.o $(BIN)
