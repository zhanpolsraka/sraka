#ifndef _INTERPRET_H_
#define _INTERPRET_H_

typedef struct
{
    tData **data;
    int size;
    int top;

}tExprStack;

void execute(tInstrStack *st);
void exec_in_class(tInstrStack *st, tExprStack *exp_st, int indx);
void exec_instructions(tInstrStack *st, tExprStack *exp_st, int indx);

void control_instr(tExprStack *exp_st, tInstruction *instr, int *indx);
void assign_instr(tExprStack *st, tInstruction *instr);
void arithmetic_instr(tExprStack *st, int operation);
void logical_instr(tExprStack *st, int operation);

string *convert_to_string(tData *target);
bool convert_to_bool(tData *target);
void concate(string *str1, string *str2, string *dest);

void set_while(int beg, int end);
tExprStack *stack_expr_init();
void stack_expr_push(tExprStack *st, tData *new);
tData *stack_expr_pop(tExprStack *st);
void stack_expr_destroy(tExprStack *st);

void print_expr_stack(tExprStack *st);

#endif
