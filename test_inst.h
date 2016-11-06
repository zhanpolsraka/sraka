/** prototype
*** instruction_list.h
*** IFJ 2016
**/
#ifndef _INSTRUCTION_LIST_H_
#define _INSTRUCTION_LIST_H_

typedef enum{
/* 0  = '()'      */INSTR_CALL_FUNC = 30,
/* 1 = 'return'   */INSTR_RETURN,
/* */               INSTR_ASS_ARG,
                    INSTR_INSERT,

/* 7  = '%'       */INSTR_MOD,

/* 15 = '++'      */INSTR_INC,
/* 16 = '--'      */INSTR_DEC,

                    INSTR_BEG_COND,
/* 17 = 'if'      */INSTR_IF,
/* 18 = 'else'    */INSTR_ELSE,
/* 20 = 'for'     */INSTR_FOR,
/* 19 = 'while'   */INSTR_WHILE,

                    INSTR_END_BLCK,

/* 21 = 'break'   */INSTR_BREAK,
/* 22 = 'continue'*/INSTR_CONTINUE,
/* 23 = 'do'      */INSTR_DO,
} tOperation;

typedef enum{

    INST_CLASS,
    INST_FUNCTION,
    INST_END_CLASS,
    INST_END_FUNCTION,
    INST_INSTRUCTION

}tInstanceType;

typedef struct instruction {

    int op;
    void *addr1;
    void *addr2;
    void *addr3;

}tInstruction;


typedef struct{

    string *name;
    tInstruction *instr;
    tInstanceType type;

}tInstance;

typedef struct{

    tInstance **inst;
    int top;
    int size;

}tInstrStack;

bool instr_stack_push(tInstrStack *st, tInstance *data);
tInstance *instr_stack_pop(tInstrStack *st);
void instr_free_stack(tInstrStack *st);
void instr_stack_destroy(tInstrStack *st);
bool instr_stack_full(tInstrStack *st);
bool instr_stack_empty(tInstrStack *st);
bool instr_stack_init(tInstrStack *st);
void print_instr_stack(tInstrStack *st);

bool create_instance(string *name, int type);
bool create_instruction(int op, void *addr1, void *addr2, void *addr3);
bool make_relations(tInstrStack *st);
bool is_id(string *str);
int get_type(string *str);
bool is_integer(string *string);
bool is_double(string *string);
bool is_string(string *string);
bool is_boolean(string *string);

bool reverse_instr_stack(tInstrStack *st);

#endif
