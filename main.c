
#include <stdio.h>
#include <stdbool.h>
#include "str.h"
#include "test_error.h"
#include "test_scanner.h"
#include "test_table.h"
#include "test_parser_remake.h"


int main()
{
    // otevirame zdrojovy soubor
    open_source("text.txt");
    // inicializuje tabulku symbolu
    tTree tree;
    tree_init(&tree);
    // inicializujeme seznam instrukci
    tInstrStack instr;
    instr_stack_init(&instr);

    // provadime parsing
    parsing();

    reverse_instr_stack(&instr);
    print_instr_stack(&instr);

    printf("%s\n",     "---------------MAKE RELATIONS--------------");
    make_relations(&instr);
    printf("                     %s\n", "*");
    printf("                     %s\n", "*");
    printf("                     %s\n", "*");
    printf("%s\n",     "------------------EXECUTE------------------");
    printf("\n%s\n\n", "------------------OUTPUT-------------------");
    printf("\n%s\n\n", "-----------------END OUTPUT----------------");

    printf("\n%s\n\n", "----------------ALL RIGHT!!!---------------");

    printf("%s\n\n",   "----------------SYMBOL TREE----------------");
    print_tree(tree.root);
    printf("\n");
    printf("\n%s",     "--------------INSTRUCTION LIST-------------" );
    print_instr_stack(&instr);
    printf("\n");

    // uvolneme tabulku symbolu a seznam instrukci
	destroy_tree(tree.root);
    // zavreme zdrojovy soubor
	close_source();

    return 0;
}
