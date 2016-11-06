#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>

#include "str.h"
#include "test_scanner.h"
#include "test_error.h"

char * desc_error[] = {

	"Lexical error:",
	"Syntax error:",
	"Semantic error:",
	"Interpreter error:",
	"Programm error:",
	"Allocation error:",

	"Unknown class begin:",
};

char *reasons[37] = {
	"system did not allocate the memory for structures",
	"expected \"class\" keyword",
	"expected identifier",
	"class with this name already exist",
	"unknown error in binary tree",
	"function with this name already exist",
	"expected other type",
	"unknown expression",
	"unknown lexem",
	"open file error",
	"error in arguments of called function",
	"undefined variable",
	"undefined class",
	"undefined function",
	"illegal operation"
};

/*	Funkce vypise chybu na stderr */
void throw_err(tErrType err_type, int reason){

	fprintf(stderr, "\n%s %s", desc_error[err_type], reasons[reason]);
	if (err_type != SEM_ERROR)
		fprintf(stderr, " on line %d\n\n", line);
	else
		fprintf(stderr, "\n\n");
}
