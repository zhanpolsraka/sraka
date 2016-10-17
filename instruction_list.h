/** prototype
*** instruction_list.h
*** IFJ 2016
**/
#ifndef _INSTRUCTION_LIST_H_
#define _INSTRUCTION_LIST_H_

#include <stdlib.h>
#include <stdbool.h>
#include "str.h"
#include "test_scanner.h"
#include "test_table.h"

extern tClassList *g_class_list;

/**
*** Instruction which contains three-address code.
**/
typedef struct instruction tInstruction;
struct instruction {
    int line;
    tType op;
    void *first_address;
    void *second_address;
    void *third_address;
    tInstruction *next;
};

/**
*** Function instance which contains its own instructions and variables.
**/
typedef struct function tFunction;
struct function {
    string *name;
    tNode *vars;
    tInstrList *instr_list;
    tFunction *next_func;
}

/**
*** Linked instruction list.
**/
typedef struct {
    tInstruction *first;
    tInstruction *current;
} tInstrList;

/**
*** Linked function list.
**/
typedef struct {
    tFunction *first;
    tFunction *current;
} tFuncList;

/**
*** Class which contains its own instructions&functions and pointers to another classes.
**/
typedef struct class tClass;
struct class {
    string *name;
    tNode *vars;

    tInstrList *instr_list;
    tFuncList *func_list;


    tClass *prev_class;
    tClass *next_class;
};

/**
*** Linked class list.
**/
typedef struct {
    tClass *first;
    tClass *current;
    tClass *last;
} tClassList;

/**
*** Create a new instruction class and add it into global class list.
*** WARNING: Exception handling is missing! The function returns:
*** true if everything is OK
*** false if something isn't OK (malloc error)
**/
bool gen_class(string *name);
/**
*** Create a new instruction and add it into current instruction list.
*** WARNING: Exception handling is missing! The function returns:
*** true if everything is OK
*** false if something isn't OK (malloc error)
**/
bool gen_instruction(tType operation, void *first_addr, void *second_addr, void *third_addr);
/**
*** Create a new function instance and add it into function list.
*** WARNING: Exception handling is missing! The function returns:
*** true if everything is OK
*** false if something isn't OK (malloc error)
**/
bool gen_function(string *name);

/**
*** The list "methods" for classes.
**/
bool create_and_init_g_class_list();
void destroy_g_class_list();

/**
*** The list "methods" for functions.
**/
tFuncList *create_and_init_func_list();
void destroy_func_list(tFuncList *list);

/**
*** Makes pointer to current function unavailable.
*** Call it when you're out of function.
**/
void function_out();

/**
*** The list "methods" for instructions.
**/
tInstrList *create_and_init_instr_list();
void destroy_instr_list(tInstrList *list);



#endif // _INSTRUCTION_LIST_H_
