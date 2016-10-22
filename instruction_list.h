/** prototype
*** instruction_list.h
*** IFJ 2016
**/
#ifndef _INSTRUCTION_LIST_H_
#define _INSTRUCTION_LIST_H_

#include <stdlib.h>
#include <stdbool.h>
#include "str.h"
/*#include "test_table.h"*/

typedef struct instruction tInstruction;
typedef struct function tFunction;
typedef struct instrList tInstrList;
typedef struct funcList tFuncList;
typedef struct class tClass;
typedef struct classList tClassList;
typedef enum operations {
/* 0  = '()'      */CALL_FUNC,
/* 1  = '='       */ASSIGNMENT,
/* 2  = '+'       */PLUS,
/* 3  = '-'       */MINUS,
/* 4  = '*'       */MUL,
/* 5  = '/'       */DIV,
/* 6  = '%'       */MOD,
/* 7  = '<'       */LESS,
/* 8  = '>'       */GREATER,
/* 9  = '<='      */LOEQ,
/* 10 = '>='      */GOEQ,
/* 11 = '!='      */NOTEQ,
/* 12 = '=='      */EQEQ,
/* 13 = '!'       */NOT,
/* 14 = '++'      */INC,
/* 15 = '--'      */DEC,
/* 16 = 'if'      */IF,
/* 17 = 'else'    */ELSE,
/* 18 = 'while'   */WHILE,
/* 19 = 'for'     */FOR,
/* 20 = 'break'   */BREAK,
/* 21 = 'continue'*/CONTINUE,
/* 22 = 'do'      */DO,
/* 20 = 'return'  */RETURN,
} tOperation;

extern tClassList *g_class_list;

/**
*** Instruction which contains three-address code.
**/
struct instruction {
    tOperation op;
    void *first_address;
    void *second_address;
    void *third_address;
    tInstruction *next;
};

/**
*** Function instance which contains its own instructions and variables.
**/
struct function {
    string name;
    /*tNode *vars;*/
    tInstrList *instr_list;
    tFunction *next_func;
};

/**
*** Linked instruction list.
**/
struct instrList {
    tInstruction *first;
    tInstruction *current;
};

/**
*** Linked function list.
**/
struct funcList {
    tFunction *first;
    tFunction *current;
};

/**
*** Class which contains its own instructions&functions and pointers to another classes.
**/
struct class {
    string name;
    /*tNode *vars;*/

    tInstrList *instr_list;
    tFuncList *func_list;

    tClass *prev_class;
    tClass *next_class;
};

/**
*** Linked class list.
**/
struct classList {
    tClass *first;
    tClass *current;
    tClass *last;
};

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
bool gen_instruction(tOperation operation, void *first_addr, void *second_addr, void *third_addr);
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
*** The list "methods" for instructions.
**/
tInstrList *create_and_init_instr_list();
void destroy_instr_list(tInstrList *list);


#endif // _INSTRUCTION_LIST_H_
