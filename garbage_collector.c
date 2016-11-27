#include <stdlib.h>
#include <stdbool.h>

#include "str.h"
#include "test_scanner.h"
#include "test_error.h"
#include "test_table_remake.h"
#include "test_parser_remake.h"
#include "test_precedence_remake.h"
#include "test_inst.h"
#include "frame.h"
#include "test_interpret_remake.h"
#include "in_built.h"
#include "garbage_collector.h"

// ukazatele na pomocne tokeny
Token *token = NULL;
tHelpVar *help_token = NULL;
// ukazatel na binarni strom
tTree *binary_tree = NULL;
// ukazatel na instrukcni zasobnik
//tInstrStack *instr_stack = NULL;
// ukazatel na zasobnik pro mezivysledky
tExprStack *expr_stack = NULL;
// ukazatel na zasobnik ramce lokalnich promennych
tFrameStack *frame_stack = NULL;

// funkce nastavi ukazatel na pomocne tokeny
void mark_tokens(Token *main_token, tHelpVar *h_token)
{
    token = main_token;
    help_token = h_token;
}

// funkce nastavi ukazatel na binarni strom
void mark_binary_tree(tTree *tree)
{
    binary_tree = tree;
}

// // funkce nastavi ukazatel na instrukcni zasobnik
// void mark_instr_stack(tInstrStack *st)
// {
//     instr_stack = st;
// }

// funkce nastaivi ukazatel na zasobnik pro mezivysledky
void mark_expr_stack(tExprStack *st)
{
    expr_stack = st;
}

// funkce nastavi ukazatel na zasobnik ramce lokalnich promennych
void mark_frame_stack(tFrameStack *st)
{
    frame_stack = st;
}

// funkce uvolni vsechny nastavene struktury
void free_all()
{
    // uvolnime zasobnik ramce lokalnich promennych
    if (frame_stack)
    {
        while (frame_stack->top != -1)
            destroy_frame(frame_stack);
        free(frame_stack->stack);
        free(frame_stack);
    }

    // uvolneme zasobnik pro mezivysledky
    if (expr_stack)
    {
        stack_expr_destroy(expr_stack);
    }
    // uvolneme binarni strom
    if (binary_tree)
    {
        destroy_tree(binary_tree->root);
    }
    // uvolneme pomocne tokeny
    if (token)
    {
        strFree(&help_token->identifier);
    	strFree(&token->attr);
    	free(token);
    }
}
