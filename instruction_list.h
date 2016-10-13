/** prototype
*** instruction_list.h
*** IFJ 2016
**/
#ifndef _INSTRUCTION_LIST_H_
#define _INSTRUCTION_LIST_H_

#include <stdlib.h>
#include "str.h"

#define SUCCESS 1
#define FAILURE 0

/**
*** Operations.
**/
typedef enum {
/* 0 => '=' */   ASSIGNMENT,
/* 1 => '+' */   PLUS,
/* 2 => '-' */   MINUS,
/* 3 => '*' */   MUL,
/* 4 => '/' */   DIV,
/* 5 => '<' */   LESS,
/* 6 => '>' */   GREATER,
/* 7 => '<=' */   LOEQ,
/* 8 => '>=' */   GOEQ,
/* 9 => '!' */   NOT,
/* 10 => '&&' */
/* 11=> '||' */
/* 12=> '^' */   NEQ,
/* 13 => '++' */   INCREMENT,
/* 14=> '--' */  DECREMENT,
/* 15 => '%' */   MOD
} t_operation;

/**
*** Instruction which contains three-address code.
**/
typedef struct instruction t_instruction;
struct instruction {
    t_operation op;
    void *first_address;
    void *second_address;
    void *third_address;
    t_instruction *next;
};

/**
*** Linked instruction list.
**/
typedef struct {
    t_instruction *first;
    t_instruction *current;
} t_instr_list;

/**
*** Class which contains its own instructions and pointers to another classes.
**/
typedef struct class t_instr_class;
struct class {
    string *name;
    t_instr_list *instr_list;
    t_instr_class *prev_class;
    t_instr_class *next_class;
};

/**
*** Doubly linked class list.
**/
typedef struct {
    t_instr_class *first;
    t_instr_class *current;
    t_instr_class *last;
} t_class_list;

/**
*** Create a new instruction class and add it into instr. class list.
*** WARNING: Exception handling is missing! The function returns:
*** 1 if everything is OK
*** 0 if something isn't OK (malloc error)
**/
int gen_instr_class(t_class_list *list, string *name, t_instr_list *instr_list);
/**
*** Create a new instruction and add it into instruction list.
*** WARNING: Exception handling is missing! The function returns:
*** 1 if everything is OK
*** 0 if something isn't OK (malloc error)
**/
int gen_instruction(t_instr_list *list, t_operation op, void *first_addr, void *second_addr, void *third_addr);


/**
*** The list "methods" for classes.
**/
t_class_list *create_and_init_class_list();
void destroy_class_list(t_class_list *list);

/**
*** The list "methods" for instructions.
**/
t_instr_list *create_and_init_instr_list();
void destroy_instr_list(t_instr_list *list);

#endif // _INSTRUCTION_LIST_H_
