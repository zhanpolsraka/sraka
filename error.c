/* **************************************************************************/
/* Projekt:             Implementace interpretu jazyka IFJ16				*/
/* Predmet:             Formalni jazyky a prekladace (IFJ)					*/
/* Soubor:              error.c  (Vypis chybovych hlasek)					*/
/*																			*/
/* Autor login:      	Ermak Aleksei		xermak00						*/
/*                     	Khaitovich Anna		xkhait00						*/
/*						Nesmelova Antonina	xnesmel00						*/
/*						Fedorenko Oleh		xfedor07						*/
/*						Fedin Evgenii		xfedin00						*/
/* **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>

#include "str.h"
#include "scanner.h"
#include "error.h"
#include "table.h"
#include "parser.h"
#include "precedence.h"
#include "instructions.h"
#include "frame.h"
#include "interpret.h"
#include "built_in.h"
#include "buffer.h"

char * desc_error[12] = {

	0,
	"Lexical error!",
	"Syntax error!",
	"Semantic error!",
	"Semantic error type compatibility!",
	0,
	"Semantic error!",
	"Number read error!",
	"Uninitialized variable!",
	"Divide on zero!",
	"Unknown error!"

};

char *reasons[37] = {

	" System did not allocate the memory for structure!",
	" Unknown lexem \"",
	" Error in arguments of called function with name \"",
	" Expected other type!",
	" Unknown expression with token \"",
	" Class with this name exist already: \"",
	" Function with this name exist already: \"",
	" Variable with this name exist already: \"",
	" Expected \"",
	" Variable or value have unexpected type!",
	" Uninitialized variable with name \"",
	" Undefined variable with name \"",
	" Undefined class with name \"",
	" Undefined function \"",
	" Illegal operation!",
	" Expected return value, but function returns value with other type or return nothing!"
};

/*	Funkce vypise chybu na stderr */
void throw_err(tErrType err_type, int reason, char *addition)
{
	if (err_type == 99)
	{
		fprintf(stderr, "\n%s", "Internal error!");
		if (reason)
			fprintf(stderr, "%s\n\n", reasons[reason]);
		else
			fprintf(stderr, "\n\n");
	}
	else
		fprintf(stderr, "\n%s%s", desc_error[err_type], reasons[reason]);

	if (addition)
		fprintf(stderr, "%s\"!\n\n", addition);
	else
		fprintf(stderr, "\n\n");

	close_source();
	clear_buffer();
	exit(err_type);
}
