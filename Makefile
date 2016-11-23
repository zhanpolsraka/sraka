CFLAGS=-std=c11 -Wall -Wextra -pedantic -g

BIN=ifj16

CC=gcc
TEST=test.py
RM=rm -f

ALL: str.o test_scanner.o test_error.o test_precedence_remake.o  test_table.o test_inst.o test_parser_remake.o interpret.o main.o
	$(CC) $(CFLAGS) -o $(BIN) str.o test_scanner.o test_error.o test_precedence_remake.o  test_table.o test_inst.o test_parser_remake.o interpret.o main.o

run:
	./$(BIN)
test:
	make
	./$(TEST) all
clean:
	$(RM) *.o $(BIN) tests/logs/*