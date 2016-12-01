/** prototype
*** instruction_list.h
*** IFJ 2016
**/

#ifndef _INSTRUCTION_LIST_H_
#define _INSTRUCTION_LIST_H_

typedef enum{
/* 0  = '()'      */INSTR_CALL_FUNC = 35, // addr3 - индекс положки в стэке с началом вызываемой функции
/* */               INSTR_ASS_ARG,
                    INSTR_INSERT,  // addr2 - указатель на tData значения для помещения его в стэк, addr1 - название переменной
                    INSTR_BEG_COND, // начало условия для if и while
/* 17 = 'if'      */INSTR_IF, // addr1 - индекс положки конца if, addr2 - индекс начала положки else (может не быть)
/* 18 = 'else'    */INSTR_ELSE, //addr1 - индекс конца блока else
/* 22 = 'continue'*/INSTR_CONTINUE, // addr1 - индекс положки INSTR_BEG_COND
/* 21 = 'break'   */INSTR_BREAK,    // addr1 - индекс положки конца блока while
                    INSTR_END_BLCK, // конец блока
/* 19 = 'while'   */INSTR_WHILE,    // addr1 - индекс положки INSTR_BEG_COND, addr2 - индекс положки конца блока while
                    INSTR_RETURN,

/* 23 = 'do'      */INSTR_PRINT, // встроенная функция ifj16.print()
                    INSTR_R_INT, // встроенная функция ifj16.readInt()
                    INSTR_R_DOUBLE, // встроенная функция ifj16.readDouble()
                    INSTR_R_STRING, // встроенная функция ifj16.readString()

                    INSTR_LENGTH, // встроенная функция ifj16.length()
                    INSTR_SUBSTR, // встроенная функция ifj16.substr()
                    INSTR_COMPARE, // встроенная функция ifj16.compare()
                    INSTR_SORT, // встроенная функция ifj16.sort()
                    INSTR_FIND,
                    INSTR_CALL_EXP_FUNC,// addr3 - индекс положки в стэке с началом вызываемой функции

                    CREATE_FRAME, // addr1 - узел функции
                    DEFINE_VAR // addr1 - название переменной
} tOperation;

typedef enum{

    INST_CLASS, //
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
void proc_call_func(tInstrStack *st, tInstruction *instr, int type);
void proc_ride_struct(tInstrStack *st, tInstruction *instr, int beg_indx);
string *split_name(string *id);

int get_type(string *str);
bool is_id(string *str);
bool is_integer(string *string);
bool is_double(string *string);
bool is_string(string *string);
bool is_symb(string *string);
bool is_boolean(string *string);

void reverse_instr_stack(tInstrStack *st);

#endif
