#include <stdio.h>
#include <stdlib.h>
#include "test_error.h"

char * desc_error[] = {
	"Lexical error in ",
	"Syntax error in ",
	"Semantic error in ",
	"Interpreter error in ",
	"Programm error in ",
	"Allocation error in "
};
void printErr(tErrType t_err, char * str, int line){
	fprintf(stderr, "%s'%s' on %d line!\n", desc_error[t_err], str, line);
}