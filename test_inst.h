/** prototype
*** instruction_list.h
*** IFJ 2016
**/
#ifndef _INSTRUCTION_LIST_H_
#define _INSTRUCTION_LIST_H_

typedef enum{
/* 0  = '()'      */INSTR_CALL_FUNC = 35,
/* 1 = 'return'   */INSTR_RETURN,
/* */               INSTR_ASS_ARG,
                    INSTR_INSERT,

/* 7  = '%'       */INSTR_MOD,

/* 15 = '++'      */INSTR_INC,
/* 16 = '--'      */INSTR_DEC,

                    INSTR_BEG_COND,
/* 17 = 'if'      */INSTR_IF,
/* 18 = 'else'    */INSTR_ELSE,
/* 22 = 'continue'*/INSTR_CONTINUE,
/* 21 = 'break'   */INSTR_BREAK,
/* 19 = 'while'   */INSTR_WHILE,

                    INSTR_END_BLCK,

/* 23 = 'do'      */INSTR_PRINT,
                    INSTR_R_INT,
                    INSTR_R_DOUBLE,
                    INSTR_R_STRING,

                    INSTR_LENGTH,
                    INSTR_SUBSTR,
                    INSTR_COMPARE,
                    INSTR_SORT
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

    string *name;           // maji jenom tridy a funkci
    tInstruction *instr;    // maji jenom samotne instrukce
    tInstanceType type;     // typ instanci

}tInstance;

typedef struct{

    tInstance **inst;
    int top;
    int size;

}tInstrStack;

extern char *name_inst[];
void instr_stack_push(tInstrStack *st, tInstance *data);
tInstance *instr_stack_pop(tInstrStack *st);
void instr_free_stack(tInstrStack *st);
void instr_stack_destroy(tInstrStack *st);
bool instr_stack_full(tInstrStack *st);
bool instr_stack_empty(tInstrStack *st);
void instr_stack_init(tInstrStack *st);
void print_instr_stack(tInstrStack *st);

void create_instance(string *name, int type);
void create_instruction(int op, void *addr1, void *addr2, void *addr3);
void make_relations(tInstrStack *st);
void proc_call_func(tInstrStack *st, tInstruction *instr);
void proc_ride_struct(tInstrStack *st, tInstruction *instr, int beg_indx);

int get_type(string *str);
bool is_id(string *str);
bool is_integer(string *string);
bool is_double(string *string);
bool is_string(string *string);
bool is_symb(string *string);
bool is_boolean(string *string);

void reverse_instr_stack(tInstrStack *st);

#endif
