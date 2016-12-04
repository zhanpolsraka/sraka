/* **************************************************************************/
<<<<<<< HEAD
/* Projekt:             Implementace interpretu jazyka IFJ16				*/
/* Predmet:             Formalni jazyky a prekladace (IFJ)					*/
/* Soubor:              interpret.c  (Interpretator)	       				*/
/*																			*/
/* Autor login:      	Ermak Aleksei		xermak00						*/
/*                     	Khaitovich Anna		xkhait00						*/
/*						Nesmelova Antonina	xnesmel00						*/
/*						Fedorenko Oleh		xfedor07						*/
/*						Fedin Evgenii		xfedin00						*/
=======
/* Projekt:             Implementace interpretu jazyka IFJ16		    */
/* Predmet:             Formalni jazyky a prekladace (IFJ)		    */
/* Soubor:              interpret.c  (Interpretator)	       		    */
/*							  		    */
/* Autor login:      	Ermak Aleksei		xermak00		    */
/*                     	Khaitovich Anna		xkhait00		    */
/*			Nesmelova Antonina	xnesmel00		    */
/*			Fedorenko Oleg		xfedor00		    */
/*			Fedin Evgenii		xfedin00		    */
>>>>>>> origin/master
/* **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "str.h"
#include "instructions.h"
#include "scanner.h"
#include "table.h"
#include "error.h"
#include "frame.h"
#include "interpret.h"
#include "built_in.h"
#include "buffer.h"

/* pointer on a stack frames of local variable */
tFrameStack *frames = NULL;
/* number of succesfull if-condition verification */
int if_succ = 0;
/* number of blocks */
int numb_block = 0;
/* arrays of while blocks indexes */
int *while_beg;
int *while_end;
/* place allocated for indexes */
int allocate_for_while = 3;
/* index of last while block */
int last_while_index = 0;
/* counter of called arguments */
int call_arg = 0;

/**
* Main function of interpret, executes instructions.
* @param  st    pointer to an instruction stack
*/
void execute(tInstrStack *st)
{
    /* create the frame stack */
    frames = create_frame_stack();
    /* initialize the stack for interim results */
    tExprStack *expr_stack = stack_expr_init();
    /* helping pointers */
    tNode *main_node;
    tNode *run_node;
    /* execute instruction in class body */
    int top = st->top;
    /* starting from the beginning of the stack */
    while (top > -1)
    {
        /* looking for class */
        if (st->inst[top]->type == INST_CLASS)
        {
            /* save the address of the node "Main" */
            if (equal_str(st->inst[top]->name->str, "Main"))
                main_node = get_node(st->inst[top]->name, CLASS, NULL);
            /* create the frame of local variables */
            create_frame(frames, NULL, get_node(st->inst[top]->name, CLASS, NULL), NULL);
            /* execute instructions */
            exec_in_class(st, expr_stack, --top);
        }
        /* go to the next instruction */
        top--;
    }

    /* allocate the place for indexes of while blocks */
    if (while_beg == NULL)
        if ((while_beg = malloc(sizeof(int) * allocate_for_while)) == NULL ||
            (while_end = malloc(sizeof(int) * allocate_for_while)) == NULL)
            throw_err(INT_ERROR, ALL_STRUCT, NULL);
    mark_mem(while_beg);
    mark_mem(while_end);
    while_beg[0] = -1;
    while_end[0] = -1;

    /* execute function "run" */
    top = st->top;
    /* starting from the beginning of the stack */
    while (top > -1)
    {
        /* looking for instance of the beginning of function "run" */
        if (st->inst[top]->type == INST_FUNCTION &&
            equal_str(st->inst[top]->name->str, "run"))
        {
            /* looking for the node "run" */
            run_node = get_node(st->inst[top]->name, FUNCTION, main_node->functions);
            /* create the frame of local variable */
            create_frame(frames, &main_node->key, run_node, NULL);
            /* set indexes -1 for ending of execution on the end of function "run" */
            set_ret_indx(-1);
            /* execute instructions */
            exec_instructions(st, expr_stack, --top);
            break;
        }
        /* go to the next instruction */
        top--;
    }
    //print_expr_stack(expr_stack);
}

// funkce provadi instrukci ve tridach
/**
* Function executes instructions in class.
* @param  st        pointer to an instruction stack
* @param  exp_st    pointer to a stack for interm results
* @param  indx      the index of actual instruction
*/
void exec_in_class(tInstrStack *st, tExprStack *exp_st, int indx)
{
    /* execute instruction to the end of the stack */
    while (st->inst[indx]->type != INST_END_CLASS)
    {
        /* skip function body */
        if (st->inst[indx]->type == INST_FUNCTION)
        {
            while (st->inst[indx]->type != INST_END_FUNCTION)
                indx--;
            indx--;
            continue;
        }
        /* in class execute only arithmetical, logical instructions, */
        /* assignment and definition instructions */
        else if (st->inst[indx]->type == INST_INSTRUCTION)
        {
            int operation = st->inst[indx]->instr->op;
            /* instruction of definition of the variable */
            if (operation == DEFINE_VAR)
                def_var(st->inst[indx]->instr->addr1);
            /* asiignment instructions */
            else if (operation == INSTR_INSERT || operation == ASSIGNMENT)
                assign_instr(exp_st, st->inst[indx]->instr);
            /* arithmetical instructions */
            else if (operation >= PLUS && operation <= DIV)
                arithmetic_instr(exp_st, operation);
            /* logical instructions */
            else if ((operation >= COMPARISON && operation <= NEQ) ||
                    (operation >= OR && operation <= AND))
                logical_instr(exp_st, operation);
            /* unknown instruction -> error */
            else
            {
                //printf("--->>> exec_in_class, 1\n");
                //printf("name %s\n", name_inst[operation]);
                throw_err(SYN_ERROR, ILLEGAL_OP, 0);
            }
        }
        /* go to the next instruction */
        indx--;
    }
}

/**
* Function executes instructions in function.
* @param  st        pointer to an instruction stack
* @param  exp_st    pointer to a stack for interm results
* @param indx       the index of actual instruction
*/
void exec_instructions(tInstrStack *st, tExprStack *exp_st, int indx)
{
    // index pocatecni instrukce
    /* the index of initial instruction */
    int poz = indx;
    /* helping pointer */
    tInstruction *instr;
    int operation = -1;
    /* execute instruction */
    while (true)
    {
        instr = st->inst[poz]->instr;
        /* from item INST_END_FUNCTION don't need instruction */
        if (st->inst[poz]->type != INST_END_FUNCTION)
            operation = instr->op;

        //printf("Operation %10s, pozition [%d]\n", name_inst[operation], poz);
        /**
        * Instruction of call function.
        * addr3 - the index of item in instruction stack
        */
        if (operation == INSTR_CALL_FUNC || operation == INSTR_CALL_EXP_FUNC)
        {
            /* check if has been called all arguments of function */
            if (call_arg != 0)
                throw_err(SEM_TYPE_ERROR, CALL_FUNC_ARG,
                    st->inst[*(int *)instr->addr3]->name->str);
            /* set arguments counter on null */
            call_arg = 0;
            /* save the index where we have to come back after */
            set_ret_indx(poz);
            /* go to the function item */
            poz = *(int *)instr->addr3;
        }

        /**
        * Instruction of creating new frame for called function.
        * addr1 - pointer to a class name
        * addr2 - the node of the function in binary tree
        */
        else if (operation == CREATE_FRAME)
        {
            /* create new frame */
            create_frame(frames, instr->addr1, instr->addr2, &call_arg);
        }

        /**
        * Instance of the end of the function, check type of the return value and destroy
        * frame of local variable on the top of the frame stack, come back to the place of call.
        */
        else if (st->inst[poz]->type == INST_END_FUNCTION)
        {
            /* check return value type */
            if (get_ret_val() != VOID && (exp_st->data[exp_st->top] == NULL ||
                exp_st->data[exp_st->top]->type != get_ret_val()))
                throw_err(SEM_TYPE_ERROR, RET_VALUE, 0);

            /* if on the top of the stack for interm results is return value, allocate for its */
            /* new item to not lose its in case destruction of frame */
            if (get_ret_val() != VOID && !exp_st->data[exp_st->top]->can_free)
            {
                tData *ret_val;
                if ((ret_val = malloc(sizeof(tData))) == NULL)
                    throw_err(INT_ERROR, ALL_STRUCT, 0);
                mark_mem(ret_val);
                /* pop item from the top of the stack */
                tData *pop = stack_expr_pop(exp_st);
                /* copy data */
                *ret_val = *pop;
                ret_val->can_free = true;
                stack_expr_push(exp_st, ret_val);
            }
            // nastavime index polozky kde byla funkce volana
            /* set index of item where has been called the function */
            poz = frames->stack[frames->top]->last_pozition;
            /* destroy the frame */
            destroy_frame(frames);
            // pokud ramec patri k funkci run -> ukoncime zpracovani
            /* if last function was "run" -> end of execution */
            if (poz == -1)
                break;
        }

        /**
        * Return instruction
        * addr1 - index of the end of the function
        */
        else if (operation == INSTR_RETURN)
            poz = *(int *)instr->addr1 + 1;

        /* control instructions */
        else if (operation >= INSTR_IF && operation <= INSTR_WHILE)
            control_instr(exp_st, instr, &poz);

        /* instruction of the beginnig of the condition -> skip its */
        else if (operation == INSTR_BEG_COND)
        {
            poz--;
            continue;
        }

        /* instruction of define the variable */
        else if (operation == DEFINE_VAR)
            def_var(instr->addr1);

        /* assignment instruction */
        else if (operation == INSTR_INSERT ||
                operation == ASSIGNMENT || operation == INSTR_ASS_ARG)
            assign_instr(exp_st, instr);

        /* arithmetical instructions */
        else if (operation >= PLUS && operation <= DIV)
            arithmetic_instr(exp_st, operation);

        /* logical instructions */
        else if ((operation >= COMPARISON && operation <= NOT) ||
                (operation >= OR && operation <= AND))
            logical_instr(exp_st, operation);

        /* print instruction */
        else if (operation == INSTR_PRINT)
            print(stack_expr_pop(exp_st));

        /* instruction of reading integer value */
        else if (operation == INSTR_R_INT)
                read_int(exp_st);

        /* instruction of reading double value */
        else if (operation == INSTR_R_DOUBLE)
            read_double(exp_st);

        /* instruction of string reading */
        else if (operation == INSTR_R_STRING)
            read_string(exp_st);

        /* computing of the length of the string */
        else if (operation == INSTR_LENGTH)
            length(exp_st);

        /* return substring */
        else if (operation == INSTR_SUBSTR)
            sub_str(exp_st);

        /* string compare */
        else if (operation == INSTR_COMPARE)
            compare(exp_st);

        /* symbol sort in string */
        else if (operation == INSTR_SORT)
        {
            /* pop string from stack for interm results */
            tData *pop = stack_expr_pop(exp_st);
            if (pop->type != STRING)
                throw_err(SEM_TYPE_ERROR, CALL_FUNC_ARG, "ifj16.sort");
            /* allocate new item for result */
            tData *new;
            if ((new = malloc(sizeof(tData))) == NULL)
                throw_err(INT_ERROR, ALL_STRUCT, 0);
            mark_mem(new);
            new->can_free = true;
            new->type = STRING;
            strInit(&new->value.str);
            strWriteStr(&new->value.str, sort(pop->value.str.str));
            stack_expr_push(exp_st, new);
        }

        /* search substring */
        else if (operation == INSTR_FIND)
        {
            /* pop string from stack for interm results */
            tData *pattern = stack_expr_pop(exp_st);
            tData *string = stack_expr_pop(exp_st);
            if (pattern->type != STRING ||
                string->type != STRING)
                throw_err(SEM_TYPE_ERROR, CALL_FUNC_ARG, "ifj16.find");
            /* allocate new item for result */
            tData *new;
            if ((new = malloc(sizeof(tData))) == NULL)
                throw_err(INT_ERROR, ALL_STRUCT, 0);
            mark_mem(new);
            new->can_free = true;
            new->type = INT;
            new->value.integer = find(string->value.str.str, pattern->value.str.str);
            stack_expr_push(exp_st, new);
        }
        //printf("End operation %10s, position [%d]\n", name_inst[operation], poz);
        //print_expr_stack(exp_st);
        //printf("\n--------print---------");
        //print_frames(frames);
        //printf("------------------------\n\n");
        /* go to the next instruction */
        poz--;
    }
}

/**
* Function execute control instructions.
* @param  exp_st    stack for interm results
* @param  instr     pointer to an actual instruction
* @param  indx      pointer to an index of actual instruction
*/
void control_instr(tExprStack *exp_st, tInstruction *instr, int *indx)
{
    tData *condition = NULL;
    /* helping varizble */
    int operation = instr->op;
    /* instruction needs result of condition, which have to be on the top */
    /* of the stack for interm results */
    if (operation == INSTR_IF || operation == INSTR_WHILE)
        condition = stack_expr_pop(exp_st);
    /* check operation */
    switch (operation)
    {
        /**
        * Condition jump instruction.
        * addr1 - index of the end of the if-block
        * addr2 - possible index of the beginning else-block
        */
        case INSTR_IF:;
            // zvysime flag bloku
            /* increase number of blocks counter */
            numb_block++;
            /* condition type is STRING -> error */
            if (condition->type == STRING)
                throw_err(SEM_TYPE_ERROR, OTHER_TYPE, NULL);
            /* evaluete condition */
            if ((condition->type == BOOLEAN && condition->value.boolean) ||
                (condition->type == INT && condition->value.integer) ||
                (condition->type == DOUBLE && condition->value.real))
                /* condition is valid -> increase if_succ counter */
                if_succ++;
            /* condition isn't valid -> skip if-block */
            else
            {
                /* if else-block behind of the if-block, set instr.index */
                if (instr->addr2 != NULL)
                    *indx = *(int *)instr->addr2 + 1;
                /* else, set index of the end of if-block */
                else
                    *indx = *(int *)instr->addr1;
            }
        break;

        /**
        * "Else" instruction.
        * addr1 - index of the end of else-block
        */
        case INSTR_ELSE:;
            /* if if-block has been processed succesfull -> skip else-block */
            if (if_succ == numb_block)
                *indx = *(int *)instr->addr1;
            if_succ = numb_block;
        break;

        /* end of the block */
        case INSTR_END_BLCK:;
            /* if program is on the end of while block -> set index on the beginning of the condition */
            if (*indx == while_end[last_while_index])
            {
                *indx = while_beg[last_while_index];
                last_while_index--;
            }
            else
            {
                    if_succ--;
                    numb_block--;
            }
        break;

        /**
        * Cycle instruction.
        * addr1 - index of the beginning of condition
        * addr2 - index of the end of while-block
        */
        case INSTR_WHILE:;
            /* condition type is STRING -> error */
            if (condition->type == STRING)
                throw_err(SEM_TYPE_ERROR, OTHER_TYPE, NULL);
            /* evaluete condition */
            if ((condition->type == BOOLEAN && condition->value.boolean) ||
                (condition->type == INT && condition->value.integer) ||
                (condition->type == DOUBLE && condition->value.real))
                /* set global indexes */
                set_while(*(int *)instr->addr1, *(int *)instr->addr2);
            /* condition isn't valid -> skip block */
            else
                *indx = *(int *)instr->addr2;
        break;

        /**
        * Break instruction.
        * addr1 - index of the end of while-block
        */
        case INSTR_BREAK:;
            /* remove last index */
            last_while_index--;
            *indx = *(int *)instr->addr1;
        break;

        /**
        * Continue instruction.
        * addr1 - beginning of condition of while-block
        */
        case INSTR_CONTINUE:;
            /* come back to the beginning of condition */
            *indx = *(int *)instr->addr1;
        break;
    }
    /* free condition result if it allowed */
    if (condition && condition->can_free)
        free_pointer(condition, true);
}

/**
* Function sets indexes of the beginning a the end of while-block.
* @param  beg    index of the beginning of while-block
* @param  end    index of the end of while-block
*/
void set_while(int beg, int end)
{
    /* realloc arrays if it's few place */
    if (last_while_index == allocate_for_while - 1)
    {
        allocate_for_while++;
        free_pointer(while_beg, false);
        free_pointer(while_end, false);
        if ((while_beg = realloc(while_beg, sizeof(int) * allocate_for_while)) == NULL ||
            (while_end = realloc(while_beg, sizeof(int) * allocate_for_while)) == NULL)
            throw_err(INT_ERROR, ALL_STRUCT, NULL);
        mark_mem(while_beg);
        mark_mem(while_end);
    }
    /* set new indexes */
    last_while_index++;
    while_beg[last_while_index] = beg;
    while_end[last_while_index] = end;
}

/**
* Function executes assignment operations and operations of data transfer.
* @param  st    pointer to a stack for interm results
* @param  instr pointer to an actual instruction
*/
void assign_instr(tExprStack *st, tInstruction *instr)
{
    int operation = instr->op;
    /**
    * Transfer data to stack for interm results.
    * addr2 - possible pointer to tData of some value
    * addr3 - possible name of variable
    */
    if (operation == INSTR_INSERT)
    {
        /* if in instruction exist pointer to tData -> push its */
        if (instr->addr2 != NULL)
            stack_expr_push(st, instr->addr2);
        /* else, looking for variable in frame and push pointers to its */
        else
            stack_expr_push(st, get_value(frames, instr->addr1, false));
    }

    /**
    * Assignment instruction.
    * addr3 - name of the variable
    */
    else if (operation == ASSIGNMENT || operation == INSTR_ASS_ARG)
    {
        /* helping pointer */
        tData *destination;
        tData *source = st->data[st->top];

        /* looking for variable in frame */
        if (operation == ASSIGNMENT)
        {
            /* helping copy variable name */
            string *help;
            if ((help = malloc(sizeof(string))) == NULL)
                throw_err(INT_ERROR, ALL_STRUCT, 0);
            mark_mem(help);
            strInit(help);
            strWriteStr(help, ((string *)instr->addr3)->str);
            //printf("%s\n", help->str);
            /* simultaneously initialize destination */
            destination = get_value(frames, help, true);
            strFree(help);
            free_pointer(help, true);
        }
        else
            /* if we search the argument, use arguments counter */
            destination = get_arg(call_arg--);

        /* check destination type */
        switch (destination->type)
        {
            case INT:;
                /* check source type */
                switch (source->type)
                {
                    case INT:;
                        destination->value.integer = source->value.integer;
                    break;

                    // case DOUBLE:;
                    //     destination->value.integer = source->value.real;
                    // break;

                    case BOOLEAN:;
                        destination->value.integer = source->value.boolean;
                    break;

                    default:
                        throw_err(SEM_TYPE_ERROR, OTHER_TYPE, 0);
                }
            break;

            case STRING:;
                /* we can assignment only string to variable with this type */
                if (source->type == STRING)
                {
                    strCopyString(&destination->value.str, &source->value.str);
                    //strWriteStr(&destination->value.str, source->value.str.str);
                }
                else
                    throw_err(SEM_TYPE_ERROR, OTHER_TYPE, 0);
            break;

            case DOUBLE:;
                /* check type */
                switch (source->type)
                {
                    case INT:;
                        destination->value.real = source->value.integer;
                    break;

                    case DOUBLE:;
                        destination->value.real = source->value.real;
                    break;

                    case BOOLEAN:;
                        destination->value.real = source->value.boolean;
                    break;

                    default:
                        throw_err(SEM_TYPE_ERROR, OTHER_TYPE, 0);
                }
            break;

            case BOOLEAN:;
                /* check type */
                if (source->type == BOOLEAN)
                    destination->value.boolean = source->value.boolean;
                else
                    destination->value.boolean = convert_to_bool(source);
            break;
        }
        /* program assignments value -> pop its from stack for interm results */
        tData *pop = stack_expr_pop(st);
        /* if value is interm result -> free its */
        if (pop->can_free)
        {
            /* if value is string -> free its */
            if (pop->type == STRING)
                strFree(&pop->value.str);
            free_pointer(pop, true);
        }
    }
}

// funkce provadi aritmeticke operace
/**
* Function executes arithmetical operations.
* @param  st        pointer to an stack for interm results
* @param  operation the cod of operation
*/
void arithmetic_instr(tExprStack *st, int operation)
{
    /* arithmetical instructions need two operands */
    /* pop them from stack for interm results */
    tData *addr2 = stack_expr_pop(st);
    tData *addr1 = stack_expr_pop(st);
    /* create item for result */
    tData *addr3;
    if ((addr3 = malloc(sizeof(tData))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    mark_mem(addr3);
    /* program can free item after */
    addr3->can_free = true;
    stack_expr_push(st, addr3);

    /* check operation */
    switch (operation)
    {
        case PLUS:;
            /* add strings -> use concate functions and convert to string */
            if (addr1->type == STRING || addr2->type == STRING)
            {
                strInit(&addr3->value.str);
                if (addr1->type == STRING && addr2->type == STRING)
                    concate(&addr1->value.str, &addr2->value.str, &addr3->value.str);
                else
                {
                    string *converted;
                    if (addr1->type != STRING)
                    {
                        converted = convert_to_string(addr1);
                        concate(converted, &addr2->value.str, &addr3->value.str);
                    }
                    else
                    {
                        converted = convert_to_string(addr2);
                        concate(&addr1->value.str, converted, &addr3->value.str);
                    }
                    strFree(converted);
                    free_pointer(converted, true);
                }
                addr3->type = STRING;
            }
            else if (addr1->type == DOUBLE && addr2->type == DOUBLE)
            {
                addr3->value.real = addr1->value.real + addr2->value.real;
                addr3->type = DOUBLE;
            }
            else if (addr1->type == INT && addr2->type == INT)
            {
                addr3->value.integer = addr1->value.integer + addr2->value.integer;
                addr3->type = INT;
            }
            else if (addr1->type == DOUBLE || addr2->type == DOUBLE)
            {
                if (addr1->type == DOUBLE)
                    addr3->value.real = addr1->value.real + addr2->value.integer;
                else
                    addr3->value.real = addr1->value.integer + addr2->value.real;
                addr3->type = DOUBLE;
            }
            else
                throw_err(SEM_TYPE_ERROR, OTHER_TYPE, 0);
        break;

        case MINUS:;
            if (addr1->type == DOUBLE && addr2->type == DOUBLE)
            {
                addr3->value.real = addr1->value.real - addr2->value.real;
                addr3->type = DOUBLE;
            }
            else if (addr1->type == INT && addr2->type == INT)
            {
                addr3->value.integer = addr1->value.integer - addr2->value.integer;
                addr3->type = INT;
            }
            else if (addr1->type == DOUBLE || addr2->type == DOUBLE)
            {
                if (addr1->type == DOUBLE)
                    addr3->value.real = addr1->value.real - addr2->value.integer;
                else
                    addr3->value.real = addr1->value.integer - addr2->value.real;
                addr3->type = DOUBLE;
            }
            else
                throw_err(SEM_TYPE_ERROR, OTHER_TYPE, 0);
        break;

        case MUL:;
            if (addr1->type == DOUBLE && addr2->type == DOUBLE)
            {
                addr3->value.real = addr1->value.real * addr2->value.real;
                addr3->type = DOUBLE;
            }
            else if (addr1->type == INT && addr2->type == INT)
            {
                addr3->value.integer = addr1->value.integer * addr2->value.integer;
                addr3->type = INT;
            }
            else if (addr1->type == DOUBLE || addr2->type == DOUBLE)
            {
                if (addr1->type == DOUBLE)
                    addr3->value.real = addr1->value.real * addr2->value.integer;
                else
                    addr3->value.real = addr1->value.integer * addr2->value.real;
                addr3->type = DOUBLE;
            }
            else
                throw_err(SEM_TYPE_ERROR, OTHER_TYPE, 0);
        break;

        case DIV:;
            if (addr1->type == DOUBLE && addr2->type == DOUBLE)
            {
                if (addr2->value.real == 0)
                throw_err(DIV_ERROR, 0, 0);

                addr3->value.real = addr1->value.real / addr2->value.real;
                addr3->type = DOUBLE;
            }
            else if (addr1->type == INT && addr2->type == INT)
            {
                if (addr2->value.integer == 0)
                    throw_err(DIV_ERROR, 0, 0);

                addr3->value.integer = addr1->value.integer / addr2->value.integer;
                addr3->type = INT;
            }
            else if (addr1->type == DOUBLE || addr2->type == DOUBLE)
            {
                if ((addr2->type == INT && addr2->value.integer == 0) ||
                    (addr2->type == DOUBLE && addr2->value.real == 0))
                    throw_err(DIV_ERROR, 0, 0);

                if (addr1->type == DOUBLE)
                {
                    if (addr2->type != INT)
                        throw_err(SEM_TYPE_ERROR, TYPE_KEY, 0);
                    addr3->value.real = addr1->value.real / addr2->value.integer;
                }
                else
                {
                    if (addr1->type != INT)
                        throw_err(SEM_TYPE_ERROR, TYPE_KEY, 0);
                    addr3->value.real = addr1->value.integer / addr2->value.real;
                }
                addr3->type = DOUBLE;
            }
            else
                throw_err(SEM_TYPE_ERROR, OTHER_TYPE, 0);
        break;
    }
    /* free operands if they are interm results */
    if (addr1->can_free)
    {
        if (addr1->type == STRING)
            strFree(&addr1->value.str);
        free_pointer(addr1, true);
    }
    if (addr2->can_free)
    {
        if (addr2->type == STRING)
            strFree(&addr2->value.str);
        free_pointer(addr2, true);
    }
}

// funkce provadi logicke operace
/**
* Function execunes logical operation.
* @param  st        pointer to a stack for interm results
* @param  operation the cod of operation
*/
void logical_instr(tExprStack *st, int operation)
{
    /* almost all operations need two operands */
    tData *addr2 = stack_expr_pop(st);
    tData *addr1 = NULL;
    /* if operation isn't NOT, program needs second operand */
    if (operation != NOT)
        addr1 = stack_expr_pop(st);
    /* check operands types */
    if (addr2->type == STRING || (addr1 && addr1->type == STRING))
        throw_err(SEM_TYPE_ERROR, OTHER_TYPE, 0);
    /* create item for result */
    tData *addr3;
    if ((addr3 = malloc(sizeof(tData))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    mark_mem(addr3);
    /* programm can free item after */
    addr3->can_free = true;
    addr3->type = BOOLEAN;
    stack_expr_push(st, addr3);

    /* check operation */
    switch (operation)
    {
        /* operands can have other type than BOOLEAN -> convert them */
        case COMPARISON:;
            if (addr1->type == BOOLEAN || addr2->type == BOOLEAN)
            {
                if (addr1->type != BOOLEAN)
                    addr3->value.boolean = convert_to_bool(addr1) == addr2->value.boolean;
                else
                    addr3->value.boolean = addr1->value.boolean ==  convert_to_bool(addr2);
            }
            else if (addr1->type == INT && addr2->type == INT)
                addr3->value.boolean = addr1->value.integer == addr2->value.integer;
            else if (addr1->type == DOUBLE && addr2->type == DOUBLE)
                addr3->value.boolean = addr1->value.real == addr2->value.real;
            else if (addr1->type == DOUBLE || addr2->type == DOUBLE)
            {
                if (addr1->type == DOUBLE)
                    addr3->value.boolean = addr1->value.real == addr2->value.integer;
                else
                    addr3->value.boolean = addr1->value.integer == addr2->value.real;
            }
        break;

        case LESS:;
            if (addr1->type == BOOLEAN || addr2->type == BOOLEAN)
            {
                if (addr1->type != BOOLEAN)
                    addr3->value.boolean = convert_to_bool(addr1) < addr2->value.boolean;
                else
                    addr3->value.boolean = addr1->value.boolean <  convert_to_bool(addr2);
            }
            else if (addr1->type == INT && addr2->type == INT)
            {
                addr3->value.boolean = addr1->value.integer < addr2->value.integer;
            }
            else if (addr1->type == DOUBLE && addr2->type == DOUBLE)
                addr3->value.boolean = addr1->value.real < addr2->value.real;
            else if (addr1->type == DOUBLE || addr2->type == DOUBLE)
            {
                if (addr1->type == DOUBLE)
                    addr3->value.boolean = addr1->value.real < addr2->value.integer;
                else
                    addr3->value.boolean = addr1->value.integer < addr2->value.real;
            }
        break;

        case GREATER:;
            if (addr1->type == BOOLEAN || addr2->type == BOOLEAN)
            {
                if (addr1->type != BOOLEAN)
                    addr3->value.boolean = convert_to_bool(addr1) > addr2->value.boolean;
                else
                    addr3->value.boolean = addr1->value.boolean >  convert_to_bool(addr2);
            }
            else if (addr1->type == INT && addr2->type == INT)
                addr3->value.boolean = addr1->value.integer > addr2->value.integer;
            else if (addr1->type == DOUBLE && addr2->type == DOUBLE)
                addr3->value.boolean = addr1->value.real > addr2->value.real;
            else if (addr1->type == DOUBLE || addr2->type == DOUBLE)
            {
                if (addr1->type == DOUBLE)
                    addr3->value.boolean = addr1->value.real > addr2->value.integer;
                else
                    addr3->value.boolean = addr1->value.integer > addr2->value.real;
            }
        break;

        case LOEQ:;
            if (addr1->type == BOOLEAN || addr2->type == BOOLEAN)
            {
                if (addr1->type != BOOLEAN)
                    addr3->value.boolean = convert_to_bool(addr1) <= addr2->value.boolean;
                else
                    addr3->value.boolean = addr1->value.boolean <=  convert_to_bool(addr2);
            }
            else if (addr1->type == INT && addr2->type == INT)
                addr3->value.boolean = addr1->value.integer <= addr2->value.integer;
            else if (addr1->type == DOUBLE && addr2->type == DOUBLE)
                addr3->value.boolean = addr1->value.real <= addr2->value.real;
            else if (addr1->type == DOUBLE || addr2->type == DOUBLE)
            {
                if (addr1->type == DOUBLE)
                    addr3->value.boolean = addr1->value.real <= addr2->value.integer;
                else
                    addr3->value.boolean = addr1->value.integer <= addr2->value.real;
            }
        break;

        case GOEQ:;
            if (addr1->type == BOOLEAN || addr2->type == BOOLEAN)
            {
                if (addr1->type != BOOLEAN)
                    addr3->value.boolean = convert_to_bool(addr1) >= addr2->value.boolean;
                else
                    addr3->value.boolean = addr1->value.boolean >=  convert_to_bool(addr2);
            }
            else if (addr1->type == INT && addr2->type == INT)
                addr3->value.boolean = addr1->value.integer >= addr2->value.integer;
            else if (addr1->type == DOUBLE && addr2->type == DOUBLE)
                addr3->value.boolean = addr1->value.real >= addr2->value.real;
            else if (addr1->type == DOUBLE || addr2->type == DOUBLE)
            {
                if (addr1->type == DOUBLE)
                    addr3->value.boolean = addr1->value.real >= addr2->value.integer;
                else
                    addr3->value.boolean = addr1->value.integer >= addr2->value.real;
            }
        break;

        case NEQ:;
            if (addr1->type == BOOLEAN || addr2->type == BOOLEAN)
            {
                if (addr1->type != BOOLEAN)
                    addr3->value.boolean = convert_to_bool(addr1) != addr2->value.boolean;
                else
                    addr3->value.boolean = addr1->value.boolean !=  convert_to_bool(addr2);
            }
            else if (addr1->type == INT && addr2->type == INT)
                addr3->value.boolean = addr1->value.integer != addr2->value.integer;
            else if (addr1->type == DOUBLE && addr2->type == DOUBLE)
                addr3->value.boolean = addr1->value.real != addr2->value.real;
            else if (addr1->type == DOUBLE || addr2->type == DOUBLE)
            {
                if (addr1->type == DOUBLE)
                    addr3->value.boolean = addr1->value.real != addr2->value.integer;
                else
                    addr3->value.boolean = addr1->value.integer != addr2->value.real;
            }
        break;

        case NOT:;
            if (addr2->type == BOOLEAN)
                addr3->value.boolean = !addr2->value.boolean;
            else if (addr2->type == INT)
                addr3->value.boolean = !addr2->value.integer;
            else if (addr2->type == DOUBLE)
                addr3->value.boolean = !addr2->value.real;
        break;

        case OR:;
            if (addr1->type == BOOLEAN || addr2->type == BOOLEAN)
            {
                if (addr1->type != BOOLEAN)
                    addr3->value.boolean = convert_to_bool(addr1) || addr2->value.boolean;
                else
                    addr3->value.boolean = addr1->value.boolean ||  convert_to_bool(addr2);
            }
            else if (addr1->type == INT && addr2->type == INT)
                addr3->value.boolean = addr1->value.integer || addr2->value.integer;
            else if (addr1->type == DOUBLE && addr2->type == DOUBLE)
                addr3->value.boolean = addr1->value.real || addr2->value.real;
            else if (addr1->type == DOUBLE || addr2->type == DOUBLE)
            {
                if (addr1->type == DOUBLE)
                    addr3->value.boolean = addr1->value.real || addr2->value.integer;
                else
                    addr3->value.boolean = addr1->value.integer || addr2->value.real;
            }
        break;

        case AND:;
            if (addr1->type == BOOLEAN || addr2->type == BOOLEAN)
            {
                if (addr1->type != BOOLEAN)
                    addr3->value.boolean = convert_to_bool(addr1) && addr2->value.boolean;
                else
                    addr3->value.boolean = addr1->value.boolean &&  convert_to_bool(addr2);
            }
            else if (addr1->type == INT && addr2->type == INT)
                addr3->value.boolean = addr1->value.integer && addr2->value.integer;
            else if (addr1->type == DOUBLE && addr2->type == DOUBLE)
                addr3->value.boolean = addr1->value.real && addr2->value.real;
            else if (addr1->type == DOUBLE || addr2->type == DOUBLE)
            {
                if (addr1->type == DOUBLE)
                    addr3->value.boolean = addr1->value.real && addr2->value.integer;
                else
                    addr3->value.boolean = addr1->value.integer && addr2->value.real;
            }
        break;
    }
    /* free operands if they are interm results */
    if (operation != NOT && addr1->can_free)
        free_pointer(addr1, true);
    if (addr2->can_free)
        free_pointer(addr2, true);
}

/**
* Function converts data type to type STRING, return result as string.
* @param target     pointer to an item tData we want to convert
* @return           pointer to a string
*/
string *convert_to_string(tData *target)
{
    /* allocate new string for result */
    string *result;
    if ((result = malloc(sizeof(string))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    mark_mem(result);
    strInit(result);

    switch (target->type)
    {
        case INT:;
            char int_n[50];
            sprintf(int_n, "%d", target->value.integer);
            strWriteStr(result, int_n);
        break;

        case DOUBLE:;
            char db_n[50];
            sprintf(db_n, "%g", target->value.real);
            strWriteStr(result, db_n);
        break;

        case BOOLEAN:;
            if (target->value.boolean)
                strWriteStr(result, "true");
            else
                strWriteStr(result, "false");
        break;

        case SYMB:;
            strAddChar(result, target->value.str.str[0]);
        break;

        case STRING:;
            strCopyString(result, &target->value.str);
        break;
    }
    return result;
}

/**
* Function convert data type to type BOOLEAN, return true or false.
* @param  target    pointer to an item tData we want to convert
* @return           true or false
*/
bool convert_to_bool(tData *target)
{
    switch (target->type)
    {
        case INT:;
            return target->value.integer != 0 ? true : false;
        case DOUBLE:;
            return target->value.real != 0 ? true : false;
        default:
            throw_err(SEM_TYPE_ERROR, OTHER_TYPE, 0);
    }
    return false;
}

/**
* Function concates strings
* @param  str1  pointer to a first string
* @param  str2  pointer to a second string
* @param  dest  pointer to a string where program will write result
*/
void concate(string *str1, string *str2, string *dest)
{
    for (int i = 0; i < str1->length; i++)
        strAddChar(dest, str1->str[i]);

    for (int i = 0; i < str2->length; i++)
        strAddChar(dest, str2->str[i]);
}

/**
* Function inicialize stack for interm results
* @return       pointer on a stack
*/
tExprStack *stack_expr_init()
{
    tExprStack *st;
    if ((st = malloc(sizeof(tExprStack))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    mark_mem(st);
    st->top = -1;
	st->size = 3;
	if ((st->data = malloc(st->size * sizeof(tData *))) == NULL)
		throw_err(INT_ERROR, ALL_STRUCT, 0);
    mark_mem(st->data);
    return st;
}

/**
* Function push data on the stack
* @param  st    pointer to a stack for intrem results
* @param new    pointer to an item tData we can push
*/
void stack_expr_push(tExprStack *st, tData *new)
{
    if(!(st->top == st->size - 1))
    {
        /* there is place on stack */
        st->top++;
        st->data[st->top] = new;
    }
    else
    {
        /* there is no free place on stack -> realloc its */
        st->size += 1;
        free_pointer(st->data, false);
		if ((st->data = realloc(st->data, st->size * sizeof(tData*))) == NULL)
			throw_err(INT_ERROR, ALL_STRUCT, 0);
        mark_mem(st->data);
        stack_expr_push(st, new);
    }
}

/**
* Function pop item on the top of stack for interm results
* @param st    pointer to a stzck for interm results
*/
tData *stack_expr_pop(tExprStack *st)
{
    /* stack isn't empty */
    if (!(st->top == -1))
    {
        tData *item;
        item = st->data[st->top];
		st->data[st->top] = NULL;
		st->top--;
		return item;
    }
    else
        return NULL;
}

/**
* Function destroy stack for interm results
* @param  st    pointer to a stack for interm results
*/
void stack_expr_destroy(tExprStack *st)
{
	if (!(st->top == -1))
    {
		for (int i = st->top; i > -1; --i)
        {
			if (st->data[i] != NULL)
            {
                free_pointer(st->data[i], true);
            }
		}
	}
    free_pointer(st->data, true);
    free_pointer(st, true);
}

void print_expr_stack(tExprStack *st)
{
    for (int i = st->top; i > -1; i--)
    {
        printf("\nExpression stack item [%d] -> ", i);
        switch (st->data[i]->type)
        {
            case INT:;
                printf("typ INT, value [%d].", st->data[i]->value.integer);
            break;
            case DOUBLE:;
                printf("typ DOUBLE, value [%g].", st->data[i]->value.real);
            break;
            case BOOLEAN:;
                printf("typ BOOLEAN, value [%d].", st->data[i]->value.boolean);
            break;
            case STRING:;
                printf("typ STRING, value [%s]", strGetStr(&st->data[i]->value.str));
            break;

            default:
                printf("%s\n", "HMMMMMM.....");
        }
        printf("\n\n");
    }
}
