
#include <stdio.h>
#include <stdbool.h>
#include "str.h"
#include "test_error.h"
#include "test_scanner.h"
#include "test_table.h"
#include "test_parser_remake.h"
#include "test_inst.h"
#include "interpret.h"


int main()
{
    bool error = true;
    // otevirame zdrojovy soubor
    open_source("text.txt");
    // inicializuje tabulku symbolu
    tTree tree;
    tree_init(&tree);
    // inicializujeme seznam instrukci
    tInstrStack instr;
    instr_stack_init(&instr);

    // provadime parsing
    if (parsing_succesful())
    {
        reverse_instr_stack(&instr);
        //print_instr_stack(&instr);
        if (make_relations(&instr))
        {
            execute(&instr);
            error = false;
        }
    }
    /*
    if (!error)
    {
        printf("\n~~~Result: yes, all right~~~\n\n");

        printf("%s\n\n", "~~~~~~~~~Symbol tree~~~~~~~~~~");
        print_tree(tree.root);
        printf("\n");
        printf("%s\n", "~~~~~~Instruction list~~~~~~~~" );
        print_instr_stack(&instr);
        printf("\n");
    }
    else
    {
        printf("Result: no, something wrong\n\n");
        printf("%s\n\n", "~~~~~~~~~Symbol tree~~~~~~~~~~");
        print_tree(tree.root);
    }
    // uvolneme tabulku symbolu a seznam instrukci
	destroy_tree(tree.root);
    // zavreme zdrojovy soubor
	close_source();
*/
    return 0;
}
