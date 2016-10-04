CFLAGS=-std=c99 -Wall -Wextra -pedantic -g
NAME=interpret
CC=gcc
RM=rm -f

ALL: 
	$(CC) $(CFLAGS) -o $(NAME) $(NAME).c

clean:
	$(RM) *.o $(NAME)