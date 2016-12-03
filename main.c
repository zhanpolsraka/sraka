/* **************************************************************************/
/* Projekt:             Implementace interpretu jazyka IFJ16				*/
/* Predmet:             Formalni jazyky a prekladace (IFJ)					*/
/* Soubor:              main.c  (Hlavni modul)          					*/
/*																			*/
/* Autor login:      	Ermak Aleksei		xermak00						*/
/*                     	Khaitovich Anna		xkhait00						*/
/*						Nesmelova Antonina	xnesmel00						*/
/*						Fedorenko Oleg		xfedor00						*/
/*						Fedin Evgenii		xfedin00						*/
/* **************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include "str.h"
#include "scanner.h"
#include "table.h"
#include "parser.h"
#include "instructions.h"
#include "frame.h"
#include "interpret.h"
#include "error.h"
#include "buffer.h"

int main(int argc, const char *argv[])
{
    // vytvorime buffer
    create_buffer();
    // kontrolujeme parametry
    if (argc != 2)
        throw_err(INT_ERROR, 0, 0);
    // otevirame zdrojovy soubor
    open_source(argv[1]);
    // inicializuje tabulku symbolu
    tTree tree;
    tree_init(&tree);

    // inicializujeme seznam instrukci
    tInstrStack instr;
    instr_stack_init(&instr);
    //mark_instr_stack(&instr);

    // provadime parsing
    parsing();

    reverse_instr_stack(&instr);
    //print_instr_stack(&instr);
    //printf("%s\n",     "---------------MAKE RELATIONS--------------");
    make_relations(&instr);
    //printf("                     %s\n", "*");
    //printf("                     %s\n", "*");
    //printf("                     %s\n", "*");
    //printf("%s\n",     "------------------EXECUTE------------------");
    //printf("\n%s\n\n", "------------------OUTPUT-------------------");
    execute(&instr);
    //printf("\n%s\n\n", "-----------------END OUTPUT----------------");

    //printf("\n%s\n\n", "----------------ALL RIGHT!!!---------------");

    //printf("%s\n\n",   "----------------SYMBOL TREE----------------");
    //print_tree(tree.root);
    //printf("\n");
    //printf("\n%s",     "--------------INSTRUCTION LIST-------------" );
    //print_instr_stack(&instr);
    //printf("\n");

    // uvolneme vsechno
	//free_all();
    clear_buffer();
    // uzavreme zdrojovy soubor
	close_source();

    return 0;
}
